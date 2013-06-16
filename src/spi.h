#ifndef SPI_H
#define SPI_H

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include "types.h"

#define SPI_BITS_PER_WORD 8
#define SPI_MAX_WRITE 4096
#define SPI_DEFAULT_SPEED_HZ 8000000
#define TIMEOUT_MS 10000

void spi_transfer(int fd, u32 spi_speed_hz, u8* tx, u8* rx, u32 len);

void spi_write(int fd, u8* tx, u32 len);

void tcl_put_pixels(int fd, u8 spi_data_tx[], u16 count, pixel* pixels);

int init_spidev(char dev[], u32 spi_speed_hz);


#endif /* SPI_H */