#!/usr/bin/env python

"""A demo client for Open Pixel Control
http://github.com/zestyping/openpixelcontrol

Creates a shifting rainbow plaid pattern by overlaying different sine waves
in the red, green, and blue channels.

This version sends one frame's worth of pixels, then exits.  It also runs
at 4% of normal speed.

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

import opc
import color_utils


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

client = opc.Client(IP_PORT)
if client.can_connect():
    print '    connected to %s' % IP_PORT
else:
    # can't connect, but keep running in case the server appears later
    print '    WARNING: could not connect to %s' % IP_PORT
print


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

# how many seconds the color sine waves take to shift through a complete cycle
speed_r = 7
speed_g = -13
speed_b = 19

t = time.time() - 1370000000
t *= 0.04 # run at 4% of normal speed
pixels = []
for ii in range(n_pixels):
    pct = ii / n_pixels
    # diagonal black stripes
    pct_jittered = (pct * 77) % 37
    blackstripes = color_utils.cos(pct_jittered, offset=t*0.05, period=1, minn=-1.5, maxx=1.5)
    blackstripes_offset = color_utils.cos(t, offset=0.9, period=60, minn=-0.5, maxx=3)
    blackstripes = color_utils.clamp(blackstripes + blackstripes_offset, 0, 1)
    # 3 sine waves for r, g, b which are out of sync with each other
    r = blackstripes * color_utils.remap(math.cos((t/speed_r + pct*freq_r)*math.pi*2), -1, 1, 0, 256)
    g = blackstripes * color_utils.remap(math.cos((t/speed_g + pct*freq_g)*math.pi*2), -1, 1, 0, 256)
    b = blackstripes * color_utils.remap(math.cos((t/speed_b + pct*freq_b)*math.pi*2), -1, 1, 0, 256)
    pixels.append((r, g, b))
client.put_pixels(pixels, channel=0)

