/* Copyright 2016 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#include "cli.h"
#include "opc.h"

#define APA102_BRIGHTNESS 31  /* overall brightness level, 0 to 31 */

static u8 buffer[4 + OPC_MAX_PIXELS_PER_MESSAGE * 4 + 4];

void apa102_put_pixels(int fd, u8* buffer, u16 count, pixel* pixels) {
  int i;
  pixel* p;
  u8* d;
  u8 flag;

  d = buffer;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  for (i = 0, p = pixels; i < count; i++, p++) {
    *d++ = 0xe0 + APA102_BRIGHTNESS;
    *d++ = p->b;
    *d++ = p->g;
    *d++ = p->r;
  }
  *d++ = 0xff;
  *d++ = 0xff;
  *d++ = 0xff;
  *d++ = 0xff;
  spi_write(fd, buffer, d - buffer);
}

int main(int argc, char** argv) {
  u16 port = OPC_DEFAULT_PORT;
  u32 spi_speed_hz = 8000000;
  char* spi_device_path = "/dev/spidev1.0";

  get_speed_and_port(&spi_speed_hz, &port, argc, argv);
  if (argc > 3) {
    spi_device_path = argv[3];
  }
  return opc_serve_main(spi_device_path, spi_speed_hz, port,
                        apa102_put_pixels, buffer);
}
