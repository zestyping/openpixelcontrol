/* Copyright 2013 Google Inc.

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include "opc.h"

#define SPI_BITS_PER_WORD 8
#define SPI_MAX_WRITE 4096
#define SPI_DEFAULT_SPEED_HZ 8000000

static int spi_fd = -1;
static u8 spi_data_tx[((1 << 16) / 3) * 4 + 5];
static u32 spi_speed_hz = SPI_DEFAULT_SPEED_HZ;

void spi_transfer(int fd, u8* tx, u8* rx, u32 len) {
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

void spi_put_pixels(int fd, u16 count, pixel* pixels) {
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

void handler(u8 address, u16 count, pixel* pixels) {
  fprintf(stderr, "%d ", count);
  fflush(stderr);
  spi_put_pixels(spi_fd, count, pixels);
}

int init_spidev() {
  int fd;
  int result;
  u8 mode = 0;
  u8 bits = SPI_BITS_PER_WORD;
  u32 speed = spi_speed_hz;

  fd = open("/dev/spidev2.0", O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed to open /dev/spidev2.0\n");
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

int main(int argc, char** argv) {
  u16 port;
  pixel diagnostic_pixel;
  time_t t;

  spi_speed_hz = argc > 1 ? atof(argv[1])*1000000 : SPI_DEFAULT_SPEED_HZ;
  port = argc > 2 ? atoi(argv[2]) : OPC_DEFAULT_PORT;
  spi_fd = init_spidev();
  if (spi_fd < 0) {
    return 1;
  }
  fprintf(stderr, "SPI speed: %.2f MHz\n", spi_speed_hz*1e-6);
  opc_source s = opc_new_source(port);
  while (s >= 0 && opc_receive(s, handler, 1000));

  t = time(NULL);
  diagnostic_pixel.r = (t % 3 == 0) ? 64 : 0;
  diagnostic_pixel.g = (t % 3 == 1) ? 64 : 0;
  diagnostic_pixel.b = (t % 3 == 2) ? 64 : 0;
  spi_put_pixels(spi_fd, 1, &diagnostic_pixel);
}
