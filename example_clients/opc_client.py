#!/usr/bin/env python
#
# Python Client library for Open Pixel Control
# David Wallace / https://github.com/longears
#
# Example:
#
#   # connect to the server
#   sock = opc_client.getSocket('127.0.0.1',7890)
#
#   # make a list of pixel colors
#   colors = [ (255,0,0), (0,255,0), (0,0,255) ]
#
#   # send the colors to channel 0
#   opc_client.sendColors(sock, 0, colors)
#

import socket

def getSocket(tcp_ip,tcp_port):
    """Given an ip address (string) and port (integer), return a socket object.
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((tcp_ip, tcp_port))
    return sock

def sendColors(sock,channel,colors):
    """Send a list of colors to the given channel, using the given socket object
    channel: an int in the range 0 - 255
    colors: a list of 3-tuples containing ints in the range 0 - 255
            example: [ (255,255,255), (0,0,0), (127,0,0) ]
    Floats will be rounded down to integers.
    Values outside the legal range will be clamped.
    """
    len_hi_byte = int( (len(colors)*3) / 256)
    len_lo_byte = (len(colors*3)) % 256
    header = chr(channel) + chr(0) + chr(len_hi_byte) + chr(len_lo_byte)
    pieces = [header]
    for r,g,b in colors:
        r = min(255,max(0,int(r)))
        g = min(255,max(0,int(g)))
        b = min(255,max(0,int(b)))
        pieces.append( chr(r) + chr(g) + chr(b) )
    command = ''.join(pieces)
    sock.send(command)

def remap(x, oldmin,oldmax, newmin,newmax):
    """Remap the number x from the range oldmin-oldmax to the range newmin-newmax
    Does not clamp values that exceed min or max.
    For example, to make a sine wave that goes from 0 to 255:
        remap( math.sin(whatever), -1,1, 0,256 )
    """
    v = (x-oldmin)/(oldmax-oldmin)
    return v*(newmax-newmin) + newmin

