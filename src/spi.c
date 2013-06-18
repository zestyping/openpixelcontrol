#include "spi.h"


void parse_args(u32* speed, u16* port, int argc, char** argv) {
  if (argc > 1) {
    *speed = strtol(argv[1], 0, 10)*1000000;
  }
  if (argc > 2) {
    *port = atoi(argv[2]);
  }
}


void spi_transfer(int fd, u32 spi_speed_hz, u8* tx, u8* rx, u32 len, u16 delay) {
  struct spi_ioc_transfer transfer;
  transfer.tx_buf = (unsigned long) tx;
  transfer.rx_buf = (unsigned long) rx;
  transfer.len = len;
  transfer.delay_usecs = delay;
  transfer.speed_hz = spi_speed_hz;
  transfer.bits_per_word = SPI_BITS_PER_WORD;
  if (ioctl(fd, SPI_IOC_MESSAGE(1), &transfer) < len) {
    fprintf(stderr, "Write failed\n");
  }
}


void spi_write(int fd, u8* tx, u32 len) {
  int block;

  while (len) {
    block = len > SPI_MAX_WRITE ? SPI_MAX_WRITE : len;
    if (write(fd, tx, block) < block) {
      fprintf(stderr, "Write failed\n");
    }
    tx += block;
    len -= block;
  }
}


int init_spidev(char dev[], u32 spi_speed_hz) {
  int fd;
  int result;
  u8 mode = 0;
  u8 bits = SPI_BITS_PER_WORD;
  u32 speed = spi_speed_hz;

  fd = open(dev, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed to open device %s\n", dev);
    return -1;
  }
  if (ioctl(fd, SPI_IOC_WR_MODE, &mode) >= 0 &&
      ioctl(fd, SPI_IOC_RD_MODE, &mode) >= 0 &&
      ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) >= 0 &&
      ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) >= 0 &&
      ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) >= 0 &&
      ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) >= 0) {
    return fd;
  }
  close(fd);
  fprintf(stderr, "Failed to set SPI parameters\n");
  return -1;
}
