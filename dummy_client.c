/* Copyright 2013 Google Inc.

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <stdio.h>
#include <string.h>
#include "opc.h"

int main(int argc, char** argv) {
  int address;
  char buffer[240000];
  char* token;
  pixel pixels[60000];
  int c, chars;
  u32 hex;
  u16 count;
  u16 port;
  opc_sink s;
  int i;
  char* sep;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <server>[:<port>]\n", argv[0]);
    return 1;
  }

  s = opc_new_sink(argv[1]);
  buffer[0] = 0;
  while (s >= 0 && fgets(buffer, 240000, stdin)) {
    c = 0;
    if (!sscanf(buffer, "%d%n", &address, &c) || !c) {
      continue;
    }
    count = 0;
    token = strtok(buffer + c, " \t\r\n");
    while (token) {
      c += chars;
      if (strlen(token) == 3) {
        hex = strtol(token, NULL, 16);
        pixels[count].r = ((hex & 0xf00) >> 8) * 0x11;
        pixels[count].g = ((hex & 0x0f0) >> 4) * 0x11;
        pixels[count].b = ((hex & 0x00f) >> 0) * 0x11;
      } else if (strlen(token) == 6) {
        hex = strtol(token, NULL, 16);
        pixels[count].r = (hex & 0xff0000) >> 16;
        pixels[count].g = (hex & 0x00ff00) >> 8;
        pixels[count].b = (hex & 0x0000ff) >> 0;
      } else {
      }
      count++;
      token = strtok(NULL, " \t\r\n");
    }
    printf("<- address %d: %d pixel%s", address, count, count == 1 ? "" : "s");
    sep = ":";
    for (i = 0; i < count; i++) {
      if (i >= 4) {
        printf(", ...");
        break;
      }
      printf("%s %02x %02x %02x", sep, pixels[i].r, pixels[i].g, pixels[i].b);
      sep = ",";
    }
    printf("\n");
    opc_put_pixels(s, address, count, pixels);
    buffer[0] = 0;
  }
}
