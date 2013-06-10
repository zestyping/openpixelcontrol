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
    ./bin/gl_server layouts/wall.l

Then run this script in another shell to send colors to the simulator

    ./example_clients/spatial_stripes.py --layout layouts/wall.l

"""

from __future__ import division
import time
import sys
import optparse

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

f = file(options.layout,'r')
coordinates = []
for line in f.readlines():
    line = line.strip()
    if line.startswith('#'): continue
    coordinates.append(tuple([float(c) for c in line.split(' ')]))


#-------------------------------------------------------------------------------
# connect to server

print '    connecting to server at %s' % options.server
print

SOCK = opc_client.get_socket(options.server)


#-------------------------------------------------------------------------------
# color function

def pixel_color(t, coord, ii, n_pixels):
    """Compute the color of a given pixel.

    t: time in seconds since the program started.
    ii: which pixel this is, starting at 0
    coord: the (x, y, z) position of the pixel as a tuple

    Returns an (r, g, b) tuple in the range 0-255

    """
    # make moving stripes for x, y, and z
    x, y, z = coord
    r = opc_client.cos(x, offset=t / 4, period=1, minn=0, maxx=0.7)
    g = opc_client.cos(y, offset=t / 4, period=1, minn=0, maxx=0.7)
    b = opc_client.cos(z, offset=t / 4, period=1, minn=0, maxx=0.7)
    r, g, b = opc_client.contrast((r, g, b), 0.5, 2)

    # make a moving white dot showing the order of the pixels in the layout file
    spark_ii = (t*80) % n_pixels
    spark_rad = 8
    spark_val = max(0, (spark_rad - opc_client.mod_dist(ii, spark_ii, n_pixels)) / spark_rad)
    spark_val = min(1, spark_val*2)
    r += spark_val
    g += spark_val
    b += spark_val

    return (r*256, g*256, b*256)


#-------------------------------------------------------------------------------
# send pixels

print '    sending pixels forever (control-c to exit)...'
print

n_pixels = len(coordinates)
start_time = time.time()
while True:
    t = time.time() - start_time
    pixels = [pixel_color(t, coord, ii, n_pixels) for ii, coord in enumerate(coordinates)]
    opc_client.put_pixels(SOCK, 0, pixels)
    time.sleep(1 / options.fps)

