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

from __future__ import print_function, division
import time
import math
import sys
import random
import argparse

import opcmatrix

#-------------------------------------------------------------------------------
# handle command line

choices = ['matrix', 'image', 'anim', 'test']

parser = argparse.ArgumentParser()
parser.add_argument('-H', dest='IP_PORT', default="127.0.0.1:7890")
parser.add_argument('-t', dest='type', required='True', choices=choices)
args = parser.parse_args()

#-------------------------------------------------------------------------------
# connect to server

client = opcmatrix.Client(args.IP_PORT)
if client.can_connect():
    print('connected to {}'.format(args.IP_PORT))
else:
    print('error: could not connect to {}'.format(args.IP_PORT))
    sys.exit(1)

#-------------------------------------------------------------------------------
# start processing

#driver = opcmatrix.TetrisMatrixDriver()
driver = opcmatrix.FreespaceMatrixDriver()
matrix = None
animated = False

if args.type == 'matrix':
    animated = True
    matrix = opcmatrix.MatrixMatrix(client, driver, 1, 10)
elif args.type == 'anim':
    animated = True
    matrix = opcmatrix.AnimatedImageMatrix(client, driver, 0)
    matrix.load('images/NyanCat.gif', 0)
elif args.type == 'image':
    matrix = opcmatrix.ImageMatrix(client, driver)
    matrix.load('images/mario1.gif', 0)
elif args.type == 'test':
    matrix = opcmatrix.TestMatrix(client, driver)


if animated:
    try:
        matrix.renderloop()
    except BaseException as e:
        print(e)
        matrix.clear()
        matrix.render()
    finally:
        client.disconnect()
        sys.exit(0)
else:
    try:
        matrix.render()
    except BaseException as e:
        print(e)
        matrix.clear()
        matrix.render()
    finally:
        client.disconnect()
        sys.exit(0)


#matrix = AnimatedImageMatrix(client, 10, 20, 0)
#try:
#    matrix.load('nyan.gif', 1)
#    matrix.renderloop()
#except BaseException as e:
#    print(e)
#    matrix.clear()
#    matrix.render()
#finally:
#    client.disconnect()
#    sys.exit(0)

#while True:
#    fps = 50
#    npix = 10
#    r = g = b = 255
#    client.put_pixels([(0,g,0) for _ in range(npix)])
#    time.sleep(1 / float(fps))
#    r = g = b = 0
#    client.put_pixels([(r,g,b) for _ in range(npix)])
#    time.sleep(1 / float(fps))
#
#client.disconnect()
#sys.exit(0)
