/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include "cli.h"
#include "spi.h"
#include "opc.h"
#include <stdlib.h>
#include <string.h>

static u8 buffers[10][4 + OPC_MAX_PIXELS_PER_MESSAGE * 4];
static int buffer_lens[10];
static int num_spi_fds = 0;
static int spi_fds[10];
static int firsts[10];
static int lasts[10];

void tcl_put_pixels(u8* dummy, u16 count, pixel* pixels) {
  int c;
  int i;
  pixel* p;
  u8* d;
  u8 flag;

  for (c = 0; c < num_spi_fds; c++) {
    d = buffers[c];
    *d++ = 0;
    *d++ = 0;
    *d++ = 0;
    *d++ = 0;
    for (i = firsts[c], p = pixels + firsts[c];
         i < count && (lasts[c] < 0 || i <= lasts[c]);
         i++, p++) {
      flag = (p->r & 0xc0) >> 6 | (p->g & 0xc0) >> 4 | (p->b & 0xc0) >> 2;
      *d++ = ~flag;
      *d++ = p->b;
      *d++ = p->g;
      *d++ = p->r;
    }
    buffer_lens[c] = d - buffers[c];
  }
  for (c = 0; c < num_spi_fds; c++) {
    spi_write(spi_fds[c], buffers[c], buffer_lens[c]);
  }
}

int parse_int(char* str, int fallback) {
  return (str != NULL && *str >= '0' && *str <= '9') ? atoi(str) : fallback;
}

void parse_channel_spec(char* spec, char** device_path, int* first, int* last) {
  char* buffer = strdup(spec);  // never freed
  char* empty = "";
  char* colon;
  char* hyphen;
  char* f;
  char* l;
  char* c;

  colon = strchr(buffer, ':');
  *device_path = buffer;
  if (colon != NULL) *colon = 0;
  f = colon == NULL ? empty : colon + 1;
  hyphen = strchr(f, '-');
  l = hyphen == NULL ? empty : hyphen + 1;
  *first = parse_int(f, 0);
  *last = parse_int(l, -1);
}

int main(int argc, char** argv) {
  u16 port = OPC_DEFAULT_PORT;
  u32 spi_speed_hz = 8000000;
  int c;

  get_speed_and_port(&spi_speed_hz, &port, argc, argv);
  if (argc > 3) {
    num_spi_fds = argc - 3;
    for (c = 0; c < num_spi_fds; c++) {
      char* device_path;
      parse_channel_spec(argv[c + 3], &device_path, &firsts[c], &lasts[c]);
      spi_fds[c] = opc_open_spi(device_path, spi_speed_hz);
      if (lasts[c] < 0) {
        fprintf(stderr, "%s: from pixel %d onward\n", device_path, firsts[c]);
      } else {
        fprintf(stderr, "%s: from pixel %d to pixel %d inclusive\n",
                device_path, firsts[c], lasts[c]);
      }
    }
  } else {
    num_spi_fds = 1;
    spi_fds[0] = opc_open_spi("/dev/spidev1.0", spi_speed_hz);
    firsts[0] = 0;
    lasts[0] = -1;
  }
  return opc_serve_main(port, tcl_put_pixels, NULL);
}
