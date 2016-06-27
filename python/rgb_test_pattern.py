#!/usr/bin/env python
"""
A demo client for Open Pixel Control
Modified from the raver_plaid client by 
    David Wallace / https://github.com/longears

Sets all pixels in strip to red, then green, then blue, then dark.
Useful for diagnosing color input order issues with LED strips that
may expect input orders other than RGB (e.g. WS2801).

Usage:
    python_clients/rgb_test_pattern.py
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
    print '    Usage: rgb_test_pattern.py [ip:port]'
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
fps = 2         # frames per second (color switches every frame)

while True:
    for c in range(4):
        pixels = []
        rgb = [ 0, 0, 0 ]
        if c < 3:
            rgb[c%3] = 255
        rgb = tuple(rgb)
        for ii in range(n_pixels):
            pixels.append(rgb)
        opc_client.put_pixels(SOCK, 0, pixels)
        time.sleep(1 / fps)

