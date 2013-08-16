/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

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
void get_speed_and_port(u32* speed, u16* port, int argc, char** argv);

void spi_transfer(int fd, u32 spi_speed_hz, u8* tx, u8* rx, u32 len, u16 delay);

void spi_write(int fd, u8* tx, u32 len);

int init_spidev(char dev[], u32 spi_speed_hz);

#endif /* SPI_H */
