#include "spi.h"
#include "opc.h"

#define POST_TX_DELAY_USECS 1000
// I'm seeing flickering at speeds above ~6mhz. Although the 
// WS2801 datasheet lists a max rate of 25mhz, anecdotal evidence 
// from forum posts suggests that most people have had trouble 
// obtaining that in practice.
#define WS2801_DEFAULT_SPEED 4000000

// Different WS2801 strips expect different input color orderings.
typedef enum { RGB=0, GRB=1, BGR=2 } order_t;
#define DEFAULT_INPUT_ORDER RGB

static u32 spi_speed_hz = WS2801_DEFAULT_SPEED;
static u8 buffer[1 << 16];
static order_t rgb_order = DEFAULT_INPUT_ORDER;
static int spi_fd;

void ws2801_put_pixels(u8 buffer[], u16 count, pixel* pixels) {
  int i;
  pixel* p;
  u8* d;

  d = buffer;
  for (i = 0, p = pixels; i < count; i++, p++) {
    switch (rgb_order)
    {
    case RGB:
      *d++ = p->r;
      *d++ = p->g;
      *d++ = p->b;
      break;
    case GRB:
      *d++ = p->g;
      *d++ = p->r;
      *d++ = p->b;
      break;
    case BGR:
      *d++ = p->b;
      *d++ = p->g;
      *d++ = p->r;
      break;
    }
  }
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
  return opc_serve_main(port, ws2801_put_pixels, buffer);
}
