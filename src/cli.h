/* Copyright 2016 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#ifndef CLI_H
#define CLI_H

#include "opc.h"

#define INACTIVITY_TIMEOUT_MS 60000
#define DIAGNOSTIC_TIMEOUT_MS 1000

// Parse command line args to get port number (argv[1]) and speed (argv[2]).
// Does not alter values if no appropriate arg is present, so default values
// should be assigned prior to calling this.
void get_speed_and_port(u32* speed, u16* port, int argc, char** argv);

// Send pixel data to LED hardware.  Caller is expected to provide a buffer
// large enough for the hardware-specific data frame for all the pixels.
typedef void put_pixels_func(int fd, u8* buffer, u16 count, pixel* pixels);

// Listen for TCP connections on the specified port, receive OPC data, and
// transmit it to the specified SPI device using the given put_pixels function.
int opc_serve_main(char* spi_device_path, u32 spi_speed_hz, u16 port,
                   put_pixels_func* put_pixels, u8* buffer);

#endif
