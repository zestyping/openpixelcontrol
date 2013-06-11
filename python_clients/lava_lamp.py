#!/usr/bin/env python
"""
A demo client for Open Pixel Control
David Wallace / https://github.com/longears

Creates moving stripes visualizing the x, y, and z coordinates
mapped to r, g, and b, respectively.  Also draws a moving white
spot which shows the order of the pixels in the layout file.

To run:
First start the gl simulator using, for example, the included "wall" layout

    make
    ./bin/gl_server layouts/wall.json

Then run this script in another shell to send colors to the simulator

    ./example_clients/spatial_stripes.py --layout layouts/wall.json

"""

from __future__ import division
import time
import sys
import optparse
import random
try:
    import json
except ImportError:
    import simplejson as json

import opc_client


#-------------------------------------------------------------------------------
# command line

parser = optparse.OptionParser()
parser.add_option('-l', '--layout', dest='layout',
                    action='store', type='string',
                    help='layout file')
parser.add_option('-s', '--server', dest='server', default='127.0.0.1:7890',
                    action='store', type='string',
                    help='ip and port of server')
parser.add_option('-f', '--fps', dest='fps', default=20,
                    action='store', type='int',
                    help='frames per second')

options, args = parser.parse_args()

if not options.layout:
    parser.print_help()
    print
    print 'ERROR: you must specify a layout file using --layout'
    print
    sys.exit(1)


#-------------------------------------------------------------------------------
# parse layout file

print
print '    parsing layout file'
print

coordinates = []
for item in json.load(open(options.layout)):
    if 'point' in item:
        coordinates.append(tuple(item['point']))


#-------------------------------------------------------------------------------
# connect to server

print '    connecting to server at %s' % options.server
print

SOCK = opc_client.get_socket(options.server)


#-------------------------------------------------------------------------------
# color function

def pixel_color(t, coord, ii, n_pixels, random_values):
    """Compute the color of a given pixel.

    t: time in seconds since the program started.
    ii: which pixel this is, starting at 0
    coord: the (x, y, z) position of the pixel as a tuple
    n_pixels: the total number of pixels
    random_values: a list containing a constant random value for each pixel

    Returns an (r, g, b) tuple in the range 0-255

    """
    # make moving stripes for x, y, and z
    x, y, z = coord
    y += opc_client.cos(x + 0.2*z, offset=0, period=1, minn=0, maxx=0.6)
    z += opc_client.cos(x, offset=0, period=1, minn=0, maxx=0.3)
    x += opc_client.cos(y + z, offset=0, period=1.5, minn=0, maxx=0.2)

    # rotate
    x, y, z = y, z, x

#     # shift some of the pixels to a new xyz location
#     if ii % 17 == 0:
#         x += ((ii*123)%5) / n_pixels * 32.12 + 0.1
#         y += ((ii*137)%5) / n_pixels * 22.23 + 0.1
#         z += ((ii*147)%7) / n_pixels * 44.34 + 0.1

    # make x, y, z -> r, g, b sine waves
    r = opc_client.cos(x, offset=t / 4, period=2, minn=0, maxx=1)
    g = opc_client.cos(y, offset=t / 4, period=2, minn=0, maxx=1)
    b = opc_client.cos(z, offset=t / 4, period=2, minn=0, maxx=1)
    r, g, b = opc_client.contrast((r, g, b), 0.5, 1.5)
#     r, g, b = opc_client.clip_black_by_luminance((r, g, b), 0.5)

#     # shift the color of a few outliers
#     if random_values[ii] < 0.03:
#         r, g, b = b, g, r

    # black out regions
    r2 = opc_client.cos(x, offset=t / 10 + 12.345, period=3, minn=0, maxx=1)
    g2 = opc_client.cos(y, offset=t / 10 + 24.536, period=3, minn=0, maxx=1)
    b2 = opc_client.cos(z, offset=t / 10 + 34.675, period=3, minn=0, maxx=1)
    clampdown = (r2 + g2 + b2)/2
    clampdown = opc_client.remap(clampdown, 0.8, 0.9, 0, 1)
    clampdown = opc_client.clamp(clampdown, 0, 1)
    r *= clampdown
    g *= clampdown
    b *= clampdown

    # color scheme: fade towards blue-and-orange
#     g = (r+b) / 2
    g = g * 0.6 + ((r+b) / 2) * 0.4

    # apply gamma curve
    # only do this on live leds, not in the simulator
    #r, g, b = opc_client.gamma((r, g, b), 2.2)

    return (r*256, g*256, b*256)


#-------------------------------------------------------------------------------
# send pixels

print '    sending pixels forever (control-c to exit)...'
print

n_pixels = len(coordinates)
random_values = [random.random() for ii in range(n_pixels)]
start_time = time.time()
while True:
    t = time.time() - start_time
    pixels = [pixel_color(t*0.6, coord, ii, n_pixels, random_values) for ii, coord in enumerate(coordinates)]
    opc_client.put_pixels(SOCK, 0, pixels)
    time.sleep(1 / options.fps)

