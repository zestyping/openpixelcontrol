#include <stdio.h>
#include "opc.h"

void handler(u8 address, u16 count, pixel* pixels) {
  int i = 0;
  char* sep = " =";
  printf("-> address %d: %d pixel%s", address, count, count == 1 ? "" : "s");
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
