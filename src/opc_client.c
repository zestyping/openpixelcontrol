/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "opc.h"

/* Wait at most 0.5 second for a connection or a write. */
#define OPC_SEND_TIMEOUT_MS 1000

#define OPC_SINK_TYPE_SOCKET 0
#define OPC_SINK_TYPE_FILE 1

#define OPC_MAX_PATH 1024

/* Internal structure for a socket sink.  sock >= 0 iff connected. */
typedef struct {
  struct sockaddr_in address;
  int sock;
  char address_string[64];
} opc_sink_socket;

/* Internal structure for a file sink.  fd >= 0 iff connected. */
typedef struct {
  int fd;
  char path[OPC_MAX_PATH + 1];
} opc_sink_file;

/* Internal structure for a sink. */
typedef struct {
  u8 type;
  union {
    opc_sink_socket socket;
    opc_sink_file file;
  } u;
} opc_sink_info;

static opc_sink_info opc_sinks[OPC_MAX_SINKS];
static opc_sink opc_next_sink = 0;

int opc_resolve(char* s, struct sockaddr_in* address, u16 default_port) {
  struct hostent* host;
  struct addrinfo* addr;
  struct addrinfo* ai;
  long port = 0;
  char* name = strdup(s);
  char* colon = strchr(name, ':');

  if (colon) {
    *colon = 0;
    port = strtol(colon + 1, NULL, 10);
  }
  getaddrinfo(colon == name ? "localhost" : name, NULL, NULL, &addr);
  free(name);
  for (ai = addr; ai; ai = ai->ai_next) {
    if (ai->ai_family == PF_INET) {
      memcpy(address, addr->ai_addr, sizeof(struct sockaddr_in));
      address->sin_port = htons(port ? port : default_port);
      freeaddrinfo(addr);
      return 1;
    }
  }
  freeaddrinfo(addr);
  return 0;
}

opc_sink opc_new_sink_socket(char* hostport) {
  opc_sink_info* info;
  opc_sink_socket* ss;

  /* Allocate an opc_sink_info entry. */
  if (opc_next_sink >= OPC_MAX_SINKS) {
    fprintf(stderr, "OPC: No more sinks available\n");
    return -1;
  }
  info = &opc_sinks[opc_next_sink];
  info->type = OPC_SINK_TYPE_SOCKET;
  ss = &(info->u.socket);
  ss->sock = -1;

  /* Resolve the server address. */
  if (!opc_resolve(hostport, &(ss->address), OPC_DEFAULT_PORT)) {
    fprintf(stderr, "OPC: Host not found: %s\n", hostport);
    return -1;
  }
  inet_ntop(AF_INET, &(ss->address.sin_addr), ss->address_string, 64);
  sprintf(ss->address_string + strlen(ss->address_string),
          ":%d", ntohs(ss->address.sin_port));

  /* Increment opc_next_sink only if we were successful. */
  return opc_next_sink++;
}

opc_sink opc_new_sink_file(char* path) {
  opc_sink_info* info;
  opc_sink_file* sf;

  if (strlen(path) > OPC_MAX_PATH) {
    fprintf(stderr, "OPC: Path is too long (max %d chars)\n", OPC_MAX_PATH);
    return -1;
  }

  /* Allocate an opc_sink_info entry. */
  if (opc_next_sink >= OPC_MAX_SINKS) {
    fprintf(stderr, "OPC: No more sinks available\n");
    return -1;
  }
  info = &opc_sinks[opc_next_sink];
  info->type = OPC_SINK_TYPE_FILE;
  sf = &(info->u.file);
  sf->fd = -1;
  strcpy(sf->path, path);

  /* Increment opc_next_sink only if we were successful. */
  return opc_next_sink++;
}

/* Backward compatibility. */
opc_sink opc_new_sink(char* hostport) {
  return opc_new_sink_socket(hostport);
}

/* Makes one attempt to connect a socket sink, returning 1 on success. */
static u8 opc_connect_socket(opc_sink_socket* ss, u32 timeout_ms) {
  int sock;
  struct timeval timeout;
  fd_set writefds;
  int opt_errno;
  socklen_t len;

  if (ss->sock >= 0) {  /* already connected */
    return 1;
  }

  /* Do a non-blocking connect so we can control the timeout. */
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  if (connect(sock, (struct sockaddr*) &(ss->address),
              sizeof(ss->address)) < 0 && errno != EINPROGRESS) {
    fprintf(stderr, "OPC: Failed to connect to %s: ", ss->address_string);
    perror(NULL);
    close(sock);
    return 0;
  }

  /* Wait for a result. */
  FD_ZERO(&writefds);
  FD_SET(sock, &writefds);
  timeout.tv_sec = timeout_ms/1000;
  timeout.tv_usec = timeout_ms % 1000;
  select(sock + 1, NULL, &writefds, NULL, &timeout);
  if (FD_ISSET(sock, &writefds)) {
    opt_errno = 0;
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &opt_errno, &len);
    if (opt_errno == 0) {
      fprintf(stderr, "OPC: Connected to %s\n", ss->address_string);
      ss->sock = sock;
      return 1;
    } else {
      fprintf(stderr, "OPC: Failed to connect to %s: %s\n",
              ss->address_string, strerror(opt_errno));
      close(sock);
      if (opt_errno == ECONNREFUSED) {
        usleep(timeout_ms*1000);
      }
      return 0;
    }
  }
  fprintf(stderr, "OPC: No connection to %s after %d ms\n",
          ss->address_string, timeout_ms);
  return 0;
}

/* Makes one attempt to open a file sink, returning 1 on success. */
static u8 opc_open_file(opc_sink_file* sf) {
  int fd;

  /* Open the file */
  fd = open(sf->path, O_CREAT | O_WRONLY | O_APPEND, 0644);
  if (fd < 0) {
    fprintf(stderr, "OPC: %s: %s", sf->path, strerror(errno));
    return 0;
  }
  sf->fd = fd;
  return 1;
}

/* Closes the connection for a sink. */
static void opc_close(opc_sink sink) {
  opc_sink_info* info = &opc_sinks[sink];

  if (sink < 0 || sink >= opc_next_sink) {
    fprintf(stderr, "OPC: Sink %d does not exist\n", sink);
    return;
  }
  switch (info->type) {
    case OPC_SINK_TYPE_SOCKET:
      if (info->u.socket.sock >= 0) {
        close(info->u.socket.sock);
        info->u.socket.sock = -1;
        fprintf(stderr, "OPC: Closed connection to %s\n",
                info->u.socket.address_string);
      }
      break;
    case OPC_SINK_TYPE_FILE:
      if (info->u.file.fd >= 0) {
        close(info->u.file.fd);
        info->u.file.fd = -1;
        fprintf(stderr, "OPC: Closed %s\n", info->u.file.path);
      }
      break;
    default:
      fprintf(stderr, "OPC: Unknown sink type %d\n", info->type);
  }
}

/* Makes one attempt to open the connection for a sink if needed, timing out */
/* after timeout_ms.  Returns 1 if connected, 0 if the timeout expired. */
static u8 opc_connect(opc_sink sink, u32 timeout_ms) {
  opc_sink_info* info = &opc_sinks[sink];

  if (sink < 0 || sink >= opc_next_sink) {
    fprintf(stderr, "OPC: Sink %d does not exist\n", sink);
    return 0;
  }
  switch (info->type) {
    case OPC_SINK_TYPE_SOCKET:
      return opc_connect_socket(&(info->u.socket), timeout_ms);
    case OPC_SINK_TYPE_FILE:
      return opc_open_file(&(info->u.file));
    default:
      fprintf(stderr, "OPC: Unknown sink type %d\n", info->type);
      return 0;
  }
}

/* Sends data to a socket sink, making at most one attempt to open the */
/* connection if needed and waiting at most timeout_ms for each I/O */
/* operation.  Returns 1 if all the data was sent, 0 otherwise. */
static u8 opc_send_socket(
    opc_sink_socket* ss, const u8* data, ssize_t len, u32 timeout_ms) {
  struct timeval timeout;
  ssize_t total_sent = 0;
  ssize_t sent;
  sig_t pipe_sig;

  timeout.tv_sec = timeout_ms/1000;
  timeout.tv_usec = timeout_ms % 1000;
  setsockopt(ss->sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  while (total_sent < len) {
    pipe_sig = signal(SIGPIPE, SIG_IGN);
    sent = send(ss->sock, data + total_sent, len - total_sent, 0);
    signal(SIGPIPE, pipe_sig);
    if (sent <= 0) {
      perror("OPC: Error sending data");
      return 0;
    }
    total_sent += sent;
  }
  return 1;
}

/* Writes data to a file sink, returning 1 if all the data was written. */
static u8 opc_write_file(opc_sink_file* sf, const u8* data, ssize_t len) {
  ssize_t total_sent = 0;
  ssize_t sent;
  sig_t pipe_sig;

  while (total_sent < len) {
    pipe_sig = signal(SIGPIPE, SIG_IGN);
    sent = write(sf->fd, data + total_sent, len - total_sent);
    signal(SIGPIPE, pipe_sig);
    if (sent <= 0) {
      perror("OPC: Error writing data");
      return 0;
    }
    total_sent += sent;
  }
  return 1;
}

/* Sends data to a sink, making at most one attempt to open the connection */
/* if needed and waiting at most timeout_ms for each I/O operation.  Returns */
/* 1 if all the data was sent, 0 otherwise. */
static u8 opc_send(opc_sink sink, const u8* data, ssize_t len, u32 timeout_ms) {
  opc_sink_info* info = &opc_sinks[sink];
  int result = 0;

  if (sink < 0 || sink >= opc_next_sink) {
    fprintf(stderr, "OPC: Sink %d does not exist\n", sink);
    return 0;
  }
  if (!opc_connect(sink, timeout_ms)) {
    return 0;
  }
  switch (info->type) {
    case OPC_SINK_TYPE_SOCKET:
      result = opc_send_socket(&(info->u.socket), data, len, timeout_ms);
      break;
    case OPC_SINK_TYPE_FILE:
      result = opc_write_file(&(info->u.file), data, len);
      break;
    default:
      fprintf(stderr, "OPC: Unknown sink type %d\n", info->type);
      return 0;
  }

  if (result == 0) opc_close(sink);
  return result;
}

u8 opc_send_header(opc_sink sink, u8 channel, u8 command, u16 len) {
  u8 header[4];

  header[0] = channel;
  header[1] = command;
  header[2] = len >> 8;
  header[3] = len & 0xff;
  return opc_send(sink, header, 4, OPC_SEND_TIMEOUT_MS);
}

u8 opc_put_pixels(opc_sink sink, u8 channel, u16 count, pixel* pixels) {
  ssize_t len;

  if (count > 0xffff / 3) {
    fprintf(stderr, "OPC: Maximum pixel count exceeded (%d > %d)\n",
            count, 0xffff / 3);
  }
  len = count * 3;

  return opc_send_header(sink, channel, OPC_SET_PIXELS, len) &&
      opc_send(sink, (u8*) pixels, len, OPC_SEND_TIMEOUT_MS);
}

u8 opc_frame_start(opc_sink sink) {
  return opc_send_header(sink, 0, OPC_FRAME_START, 0);
}

u8 opc_frame_end(opc_sink sink) {
  return opc_send_header(sink, 0, OPC_FRAME_END, 0);
}

u8 opc_stream_sync(opc_sink sink) {
  ssize_t len = OPC_STREAM_SYNC_LENGTH;
  u8* data = OPC_STREAM_SYNC_DATA;

  return opc_send_header(sink, 0, OPC_STREAM_SYNC, len) &&
      opc_send(sink, data, len, OPC_SEND_TIMEOUT_MS);
}
