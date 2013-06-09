#!/usr/bin/env python
#
# A demo client for Open Pixel Control
# David Wallace / https://github.com/longears
#
# To run:
# First start the gl simulator using the included "wall" layout
# 
#     make
#     ./bin/gl_server layouts/wall.l
# 
# Then run this script in another shell to send colors to the simulator
# 
#     ./example_clients/raver_plaid.py
#

from __future__ import division
import time
import math

import opc_client

TCP_IP = '127.0.0.1'
TCP_PORT = 7890

N_PIXELS = 1250  # number of pixels in the included "wall" layout
FPS = 20

#--------------------------------------------------------------------------------

print
print '    connecting to %s:%s'%(TCP_IP,TCP_PORT)
print

SOCK = opc_client.getSocket(TCP_IP,TCP_PORT)

print '    sending pixels forever...'
print

start_time = time.time()
while True:
    t = time.time() - start_time
    time.sleep(1/FPS)
    colors = []
    for ii in range(N_PIXELS):
        pp = ii / N_PIXELS
        r = opc_client.remap(math.cos( t*0.5 + pp * 24 * math.pi*2), -1,1, 0,256)
        g = opc_client.remap(math.cos(-t*1.1 + pp * 24 * math.pi*2), -1,1, 0,256)
        b = opc_client.remap(math.cos( t*1.7 + pp * 24 * math.pi*2), -1,1, 0,256)
        colors.append((r,g,b))
    opc_client.sendColors(SOCK,0,colors)

