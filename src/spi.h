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

// Parse command line args to extract port number (argv[1]) and speed (argv[2]).
// Does not alter values if no appropriate arg is present, so default values
// should be assigned prior to calling this.
void parse_args(u32* speed, u16* port, int argc, char** argv);

void spi_transfer(int fd, u32 spi_speed_hz, u8* tx, u8* rx, u32 len);

void spi_write(int fd, u8* tx, u32 len);

void tcl_put_pixels(int fd, u8 spi_data_tx[], u16 count, pixel* pixels);

void ws2801_put_pixels(int fd, u8 spi_data_tx[], u32 spi_speed_hz,
                       u16 count, pixel* pixels);

int init_spidev(char dev[], u32 spi_speed_hz);


#endif /* SPI_H */