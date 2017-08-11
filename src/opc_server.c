/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <strings.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif // !_WIN32
#include "compat.h"
#include "opc.h"

/* Internal structure for a source.  sock >= 0 iff the connection is open. */
typedef struct {
  u16 port;
  sock_t listen_sock;
  sock_t sock;
  u16 header_length;
  u8 header[4];
  u16 payload_length;
  u8 payload[1 << 16];
} opc_source_info;

static opc_source_info opc_sources[OPC_MAX_SOURCES];
static int opc_next_source = 0;

sock_t opc_listen(u16 port) {
  struct sockaddr_in address;
  sock_t sock;
  int one = 1;

  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&one, sizeof(one));

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  bzero(&address.sin_addr, sizeof(address.sin_addr));
  if (bind(sock, (struct sockaddr*) &address, sizeof(address)) == SOCKET_ERROR) {
    fprintf(stderr, "OPC: Could not bind to port %d: ", port);
    #ifdef _WIN32
	fprintf(stderr, "Error %d\n", WSAGetLastError());
    #else
	perror(NULL);
    #endif
    return INVALID_SOCKET;
  }
  if (listen(sock, 0) != 0) {
    fprintf(stderr, "OPC: Could not listen on port %d: ", port);
    #ifdef _WIN32
	fprintf(stderr, "Error %d\n", WSAGetLastError());
    #else
	perror(NULL);
    #endif
    return INVALID_SOCKET;
  }
  return sock;
}

opc_source opc_new_source(u16 port) {
  opc_source_info* info;

  /* Allocate an opc_source_info entry. */
  if (opc_next_source >= OPC_MAX_SOURCES) {
    fprintf(stderr, "OPC: No more sources available\n");
    return -1;
  }
  info = &opc_sources[opc_next_source];

  /* Listen on the specified port. */
  info->port = port;
  info->listen_sock = opc_listen(port);
  if (info->listen_sock == INVALID_SOCKET) {
    return -1;
  }

  /* Increment opc_next_source only if we were successful. */
  fprintf(stderr, "OPC: Listening on port %d\n", port);
  return opc_next_source++;
}

u8 opc_receive(opc_source source, opc_handler* handler, u32 timeout_ms) {
  int nfds;
  fd_set readfds;
  struct timeval timeout;
  opc_source_info* info = &opc_sources[source];
  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);
  u16 payload_expected;
  ssize_t received = 1;  /* nonzero, because we treat zero to mean "closed" */
  char buffer[64];

  if (source < 0 || source >= opc_next_source) {
    fprintf(stderr, "OPC: Source %d does not exist\n", source);
    return 0;
  }

  /* Select for inbound data or connections. */
  FD_ZERO(&readfds);
  if (info->listen_sock != INVALID_SOCKET) {
    FD_SET(info->listen_sock, &readfds);
    nfds = info->listen_sock + 1;
  } else if (info->sock != INVALID_SOCKET) {
    FD_SET(info->sock, &readfds);
    nfds = info->sock + 1;
  }
  timeout.tv_sec = timeout_ms/1000;
  timeout.tv_usec = (timeout_ms % 1000)*1000;
  select(nfds, &readfds, NULL, NULL, &timeout);
  if ((info->listen_sock != INVALID_SOCKET) && FD_ISSET(info->listen_sock, &readfds)) {
    /* Handle an inbound connection. */
    info->sock = accept(
        info->listen_sock, (struct sockaddr*) &(address), &address_len);
    inet_ntop(AF_INET, &(address.sin_addr), buffer, 64);
    fprintf(stderr, "OPC: Client connected from %s\n", buffer);
	#ifdef _WIN32
	closesocket(info->listen_sock);
	#else
    close(info->listen_sock);
	#endif
    info->listen_sock = INVALID_SOCKET;
    info->header_length = 0;
    info->payload_length = 0;
  } else if ((info->sock != INVALID_SOCKET) && FD_ISSET(info->sock, &readfds)) {
    /* Handle inbound data on an existing connection. */
    if (info->header_length < 4) {  /* need header */
      received = recv(info->sock, info->header + info->header_length,
                      4 - info->header_length, 0);
      if (received > 0) {
        info->header_length += (u16)received;
      }
    }
    if (info->header_length == 4) {  /* header complete */
      payload_expected = (info->header[2] << 8) | info->header[3];
      if (info->payload_length < payload_expected) {  /* need payload */
        received = recv(info->sock, info->payload + info->payload_length,
                        payload_expected - info->payload_length, 0);
        if (received > 0) {
          info->payload_length += (u16)received;
        }
      }
      if (info->header_length == 4 &&
          info->payload_length == payload_expected) {  /* payload complete */
        if (info->header[1] == OPC_SET_PIXELS) {
          handler(info->header[0], payload_expected/3,
                  (pixel*) info->payload);
        }
        info->header_length = 0;
        info->payload_length = 0;
      }
    }
    if (received <= 0) {
      /* Connection was closed; wait for more connections. */
      fprintf(stderr, "OPC: Client closed connection\n");
	#ifdef _WIN32
	  closesocket(info->sock);
	#else
      close(info->sock);
	#endif
      info->sock = INVALID_SOCKET;
      info->listen_sock = opc_listen(info->port);
    }
  } else {
    /* timeout_ms milliseconds passed with no incoming data or connections. */
    return 0;
  }
  return 1;
}

void opc_reset_source(opc_source source) {
  opc_source_info* info = &opc_sources[source];
  if (source < 0 || source >= opc_next_source) {
    fprintf(stderr, "OPC: Source %d does not exist\n", source);
    return;
  }

  if (info->sock >= 0) {
    fprintf(stderr, "OPC: Closed connection\n");
	#ifdef _WIN32
	closesocket(info->sock);
	#else
    close(info->sock);
	#endif
    info->sock = INVALID_SOCKET;
    info->listen_sock = opc_listen(info->port);
  }
}
