#include "spi.h"

void parse_args(u32* speed, u16* port, int argc, char** argv) {
  if (argc > 1) {
    *speed = strtol(argv[1], 0, 10)*1000000;
  }
  if (argc > 2) {
    *port = atoi(argv[2]);
  }
}

void spi_transfer(int fd, u32 spi_speed_hz, u8* tx, u8* rx, u32 len) {
  struct spi_ioc_transfer transfer;

  transfer.tx_buf = (unsigned long) tx;
  transfer.rx_buf = (unsigned long) rx;
  transfer.len = len;
  transfer.delay_usecs = 0;
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

void tcl_put_pixels(int fd, u8 spi_data_tx[], u16 count, pixel* pixels) {
  int i;
  pixel* p;
  u8* d;
  u8 flag;

  d = spi_data_tx;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  for (i = 0, p = pixels; i < count; i++, p++) {
    flag = (p->r & 0xc0) >> 6 | (p->g & 0xc0) >> 4 | (p->b & 0xc0) >> 2;
    *d++ = ~flag;
    *d++ = p->b;
    *d++ = p->g;
    *d++ = p->r;
  }
  spi_write(fd, spi_data_tx, d - spi_data_tx);
}

void ws2801_put_pixels(int fd, u8 spi_data_tx[], u32 spi_speed_hz, 
                       u16 count, pixel* pixels) {
  int i;
  pixel* p;
  u8* d;
  
  d = spi_data_tx;
  for (i = 0, p = pixels; i < count; i++) {
    *d++ = p->b;
    *d++ = p->g;
    *d++ = p->r;
  }
  spi_transfer(fd, spi_speed_hz, spi_data_tx, 0, d - spi_data_tx);
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