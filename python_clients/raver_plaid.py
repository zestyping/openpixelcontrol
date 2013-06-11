#!/usr/bin/env python
"""
A demo client for Open Pixel Control
David Wallace / https://github.com/longears

Creates a shifting rainbow plaid pattern by overlaying different sine waves
in the red, green, and blue channels.

To run:
First start the gl simulator using the included "wall" layout

    make
    bin/gl_server layouts/wall.json

Then run this script in another shell to send colors to the simulator

    python_clients/raver_plaid.py

"""

from __future__ import division
import time
import math
import sys

import opc_client


#-------------------------------------------------------------------------------
# handle command line

if len(sys.argv) == 1:
    IP_PORT = '127.0.0.1:7890'
elif len(sys.argv) == 2 and ':' in sys.argv[1] and not sys.argv[1].startswith('-'):
    IP_PORT = sys.argv[1]
else:
    print
    print '    Usage: raver_plaid.py [ip:port]'
    print
    print '    If not set, ip:port defauls to 127.0.0.1:7890'
    print
    sys.exit(0)


#-------------------------------------------------------------------------------
# connect to server

print
print '    connecting to server at %s' % IP_PORT
print

SOCK = opc_client.get_socket(IP_PORT)


#-------------------------------------------------------------------------------
# send pixels

print '    sending pixels forever (control-c to exit)...'
print

n_pixels = 1250  # number of pixels in the included "wall" layout
fps = 20         # frames per second

# how many sine wave cycles are squeezed into our n_pixels
# 24 happens to create nice diagonal stripes on the wall layout
freq_r = 24
freq_g = 24
freq_b = 24

# how many seconds the sine waves take to shift through a complete cycle
speed_r = 7
speed_g = -13
speed_b = 19

start_time = time.time()
while True:
    t = time.time() - start_time
    pixels = []
    for ii in range(n_pixels):
        pct = ii / n_pixels
        r = opc_client.remap(math.cos((t/speed_r + pct*freq_r)*math.pi*2), -1, 1, 0, 256)
        g = opc_client.remap(math.cos((t/speed_g + pct*freq_g)*math.pi*2), -1, 1, 0, 256)
        b = opc_client.remap(math.cos((t/speed_b + pct*freq_b)*math.pi*2), -1, 1, 0, 256)
        pixels.append((r, g, b))
    opc_client.put_pixels(SOCK, 0, pixels)
    time.sleep(1 / fps)

