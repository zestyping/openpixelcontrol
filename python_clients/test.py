#!/usr/bin/env python

"""A demo client for Open Pixel Control
http://github.com/zestyping/openpixelcontrol

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
    sys.exit(1)
print


#-------------------------------------------------------------------------------
# send pixels

print '    sending pixels forever (control-c to exit)...'
print

n_pixels = 200  # number of pixels in the included "wall" layout
x_pix = 10
y_pix = 20
fps = 30

import random

try:
    # clear
    pixels = []
    for i in range(n_pixels):
        pixels.append((0, 0, 0))
    client.put_pixels(pixels, channel=0)

    # do the work
    while True:
        for xp in range(x_pix) + [_ for _ in reversed(range(x_pix))]:
            ytable = range(y_pix) if xp % 2 == 0 else reversed(range(y_pix))
            for yp in ytable:
                pixels = []
                for x in range(x_pix):
                    for y in range(y_pix):
                        #powx = 1 - min(abs(x - xp), x_pix - abs(x - xp)) / x_pix
                        powx = 1 - abs(x - xp) / x_pix
                        #powy = 1 - min(abs(y - yp), y_pix - abs(y - yp)) / y_pix
                        powy = 1 - abs(y - yp) / y_pix
                        fac = powx * powy
                        if x == xp and y == yp:
                            pixels.append((255, 0, 0))
                        else:
                            pixels.append((255 * fac, 255 * fac, 255 * fac))
                client.put_pixels(pixels, channel=0)
                time.sleep(1/fps)
except:
    client.disconnect()
    raise
