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
import random

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
# Matrix

class Matrix(object):

    def __init__(self, client, width, height):
        self.width = width
        self.height = height
        self.client = client
        self.numpix = self.width * self.height
        self.data = [[(0,0,0) for _ in range(self.height)] for _ in range(self.width)]

    def fill(self, r, g, b):
        for y in range(self.height):
            for x in range(self.width):
                self.data[x][y] = (r, g, b)

    def clear(self):
        self.fill(0, 0, 0)

    def test(self):
        self.fill(255, 0, 0)

    def render(self):
        pixels = []
        for x in range(self.width):
            for y in range(self.height):
                pixels.append(self.data[x][y])
        client.put_pixels(pixels, channel=0)

class AutoMatrix(Matrix):

    def __init__(self, client, width, height, fps):
        super(AutoMatrix, self).__init__(client, width, height)
        self.fps = fps
        self.init()

    def init(self):
        pass

    def update(self):
        pass

    def renderloop(self):
        while True:
            self.update()
            self.render()
            time.sleep(1 / self.fps)

class MatrixMatrix(AutoMatrix):

    def update(self):
        for x in range(self.width):
            self.data[x].pop(0)
            if random.random() > 0.93:
                self.data[x].append((0, 255, 0))
            else:
                trio = self.data[x][-1]
                new = [int(_ * 0.99 if _ > 250 else _ * 0.75) for _ in trio]
                self.data[x].append(new)

class PongMatrix(AutoMatrix):

    class Player(object):

        def __init__(self, y, d):
            self.y = y
            self.d = d

    def init(self):
        self.psize = 4
        self.players = (PongMatrix.Player(int(self.height / 2 - self.psize / 2), 1),
                        PongMatrix.Player(int(self.height / 2 - self.psize / 2), -1))

    def update_players(self):
       for p in self.players:
            if (p.d > 0 and p.y + self.psize + p.d <= self.height) or (p.d < 0 and p.y + p.d >= 0):
                p.y += p.d
            else:
                p.d = -p.d
                p.y += p.d

    def update(self):

        self.update_players()

        for y in range(self.height):
            self.data[0][y] = (0, 0, 0)
            self.data[-1][y] = (0, 0, 0)

        for y in range(self.psize):
            self.data[0][self.players[0].y + y] = (255, 255, 255)
            self.data[-1][self.players[1].y + y] = (255, 255, 255)



#-------------------------------------------------------------------------------
# connect to server

client = opc.Client(IP_PORT)
if client.can_connect():
    print 'connected to %s' % IP_PORT
else:
    print 'error: could not connect to %s' % IP_PORT
    sys.exit(1)
print

matrix = PongMatrix(client, 10, 20, 10)

try:
    matrix.renderloop()
except:
    matrix.clear()
    matrix.render()
    raise
finally:
    client.disconnect()

