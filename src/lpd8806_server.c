#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "spi.h"
#include "opc.h"

// Driver for LED strips based on the LPD8806 chipset
// such as these sold by Adafruit
//   http://www.adafruit.com/products/306

// The official Adafruit Arduino library can be found here
//   https://github.com/adafruit/LPD8806/blob/master/LPD8806.cpp

#define POST_TX_DELAY_USECS 1000
#define LPD8806_DEFAULT_SPEED 2000000

// Different LPD8806 strips may different input color orderings.
typedef enum { RGB=0, GRB=1, BGR=2 } order_t;
#define DEFAULT_INPUT_ORDER GRB

static u8 buffer[(1 << 16) + 5];
static order_t rgb_order = DEFAULT_INPUT_ORDER;
static u32 spi_speed_hz = LPD8806_DEFAULT_SPEED;
static int spi_fd;

void lpd8806_put_pixels(u8* buffer, u16 count, pixel* pixels) {
  int i;
  pixel* p;
  u8* d;
  
  d = buffer;

  // TODO: the number of zeroes should depend on 
  //  the number of LEDs in the strip.
  // The correct number is ((numLEDs+31)/32)-1
  //  according to the Adafruit Arduino library
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;

  // Pixel data must have the high bit on.
  // The remaining 7 bits store the actual
  //  brightness value (between 0 and 127)
  for (i = 0, p = pixels; i < count; i++, p++) {
    switch (rgb_order)
    {
    case RGB:
      *d++ = 128 | ((p->r) >> 1);
      *d++ = 128 | ((p->g) >> 1);
      *d++ = 128 | ((p->b) >> 1);
      break;
    case GRB:
      *d++ = 128 | ((p->g) >> 1);
      *d++ = 128 | ((p->r) >> 1);
      *d++ = 128 | ((p->b) >> 1);
      break;
    case BGR:
      *d++ = 128 | ((p->b) >> 1);
      *d++ = 128 | ((p->g) >> 1);
      *d++ = 128 | ((p->r) >> 1);
      break;
    }
  }

  // Send one final zero to latch the last LED in the strand
  *d++ = 0;

  spi_transfer(spi_fd, spi_speed_hz, buffer, 0, d - buffer, POST_TX_DELAY_USECS);
}

order_t get_order(int argc, char** argv) {
  order_t order = DEFAULT_INPUT_ORDER;
  if (argc > 3) {
    if (!strcmp(argv[3], "rgb")) {
      order = RGB;
    } else if (!strcmp(argv[3], "grb")) {
      order = GRB;
    } else if (!strcmp(argv[3], "bgr")) {
      order = BGR;
    } else {
      fprintf(stderr, "Did not recognize color order argument - using default\n");
    }
  }
  return order;
}

int main(int argc, char** argv) {
  u16 port = OPC_DEFAULT_PORT;
  char* spi_device_path = "/dev/spidev1.0";

  get_speed_and_port(&spi_speed_hz, &port, argc, argv);
  rgb_order = get_order(argc, argv);
  spi_fd = opc_open_spi(spi_device_path, spi_speed_hz);
  return opc_serve_main(port, lpd8806_put_pixels, buffer);
}
