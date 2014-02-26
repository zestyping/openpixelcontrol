/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <stdio.h>
#include <stdlib.h>
#include "opc.h"

void handler(u8 channel, u16 count, pixel* pixels) {
  int i = 0;
  char* sep = " =";
  printf("-> channel %d: %d pixel%s", channel, count, count == 1 ? "" : "s");
  for (i = 0; i < count; i++) {
    if (i >= 4) {
      printf(", ...");
      break;
    }
    printf("%s %02x %02x %02x", sep, pixels[i].r, pixels[i].g, pixels[i].b);
    sep = ",";
  }
  printf("\n");
}

int main(int argc, char** argv) {
  u16 port = argc > 1 ? atoi(argv[1]) : OPC_DEFAULT_PORT;
  opc_source s = opc_new_source(port);
  while (s >= 0) {
    opc_receive(s, handler, 10000);
  }
}
