#!/usr/bin/env python
"""
A demo client for Open Pixel Control
David Wallace / https://github.com/longears

game of life
"""

from __future__ import division
import time
import os
import sys
import optparse
import random

import opc

# command line
parser = optparse.OptionParser()
parser.add_option('-s', '--server', dest='server', default='127.0.0.1:7890',
                    action='store', type='string',
                    help='ip and port of server')
parser.add_option('-f', '--fps', dest='fps', default=5,
                    action='store', type='int',
                    help='frames per second')
options, args = parser.parse_args()

X_DIM = 25
Y_DIM = 25

def print_board(board):
    for x in range(X_DIM):
        for y in range(Y_DIM):
            if board[x*X_DIM+y]==1:
                print 'X',
            else:
                print '.',
        print

def rand_board():
    board = ['0'] * X_DIM * Y_DIM
    for x in range(X_DIM):
        for y in range(Y_DIM):
            if not random.randint(0,3):
                board[x*X_DIM+y] = 1
    return board

def count_cell_neighbor(x,y, board):
    neighbor_count = 0
    for nei_x in range(3):
        if (x == 0 and nei_x == 0) or (x == (X_DIM-1) and nei_x == 2):
            continue
        for nei_y in range(3):
            if (y == 0 and nei_y == 0) or (y == (Y_DIM-1) and nei_y == 2):
                continue
            if board[(x-1+nei_x)*X_DIM+y-1+nei_y] == 1:
                neighbor_count += 1
    return neighbor_count

def tick(board):
    new_board = ['0'] * X_DIM * Y_DIM
    for x in range(X_DIM):
        for y in range(Y_DIM):
            n = count_cell_neighbor(x,y, board)
            if n < 2 or n > 3:
                new_board[x*X_DIM+y] = 0
            elif n == 3 or (n == 2 and board[x*X_DIM+y] == 1):
                new_board[x*X_DIM+y] = 1
    return new_board

def pixelify_board(board):
    pixels = []
    for cell in board:
        pixels.append((130, 150, 120) if cell else (0,0,0))
    return pixels

def pixelify_triboard(r,g,b):
    pixels = []
    for rcell, gcell, bcell in zip(r,g,b):
        print rcell,gcell,bcell
        color = (130 if rcell else 0, 130 if gcell else 0, 130 if bcell else 0)
        pixels.append(color)
    return pixels

board = rand_board()


client = opc.Client(options.server)
while True:
    client.put_pixels(pixelify_board(board), channel=0)
    board = tick(board)
    time.sleep(1 / options.fps)

