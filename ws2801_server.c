#include "src/spi.h"
#include "opc.h"

static int spi_fd = -1;
static u8 spi_data_tx[1 << 16];
static u32 spi_speed_hz = SPI_DEFAULT_SPEED_HZ;

void handler(u8 address, u16 count, pixel* pixels) {
  fprintf(stderr, "%d ", count);
  fflush(stderr);
  ws2801_put_pixels(spi_fd, spi_data_tx, spi_speed_hz, count, pixels);
}

int main(int argc, char** argv) {
  u16 port = OPC_DEFAULT_PORT;

  parse_args(&spi_speed_hz, &port, argc, argv);
  spi_fd = init_spidev("/dev/spidev0.0", spi_speed_hz);
  if (spi_fd < 0) {
    return 1;
  }
  fprintf(stderr, "SPI speed: %.2f MHz, ready...\n", spi_speed_hz*1e-6);
  opc_source s = opc_new_source(port);
  while (s >= 0 && opc_receive(s, handler, TIMEOUT_MS));
  fprintf(stderr, "Exiting after %d ms of inactivity\n", TIMEOUT_MS);
}