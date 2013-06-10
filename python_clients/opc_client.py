#!/usr/bin/env python

"""Python Client library for Open Pixel Control

Sends pixel values to an Open Pixel Control server to be displayed.
http://openpixelcontrol.org/

Example use:

    # connect to the server
    sock = opc_client.get_socket('127.0.0.1:7890')

    # make a list of pixel colors
    pixels = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]

    # send the pixels to channel 0
    opc_client.put_pixels(sock, 0, pixels)

Also includes some helper functions to make color manipulations easier:

    remap(x, oldmin, oldmax, newmin, newmax)
    clamp(x, min, max)
    cos(x, offset=0, period=1, minn=0, maxx=1)
    contrast(color, center, mult)
    clip_black_by_luminance(color, threshold)
    clip_black_by_channels(color, threshold)
    mod_dist(a, b, n)

"""

import math
import socket


#-------------------------------------------------------------------------------
# Communication with Open Pixel Control servers

def get_socket(ip_port):
    """Given an ip address and port as a string, return a connected socket.

    ip_port should be a string in this format: '127.0.0.1:7890'.
    You can also use a hostname: 'localhost:7890'.
    A socket.error exception will occur if the connection cannot be made.

    """
    ip, port = ip_port.split(':')
    port = int(port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    return sock

def put_pixels(sock, channel, pixels):
    """Send a list of colors to the given channel using the given socket object

    channel: Which strand of lights to send the pixel colors to.
        Must be an int in the range 0-255 inclusive.
        0 is a special value which means "all strands".

    pixels: A list of 3-tuples representing rgb colors.
        Each value in the tuple should be in the range 0-255 inclusive. 
        For example, [(255, 255, 255), (0, 0, 0), (127, 0, 0)]
        Floats will be rounded down to integers.
        Values outside the legal range will be clamped.

    A socket.error exception will occur if the connection fails.

    """
    len_hi_byte = int(len(pixels)*3 / 256)
    len_lo_byte = (len(pixels)*3) % 256
    header = chr(channel) + chr(0) + chr(len_hi_byte) + chr(len_lo_byte)
    pieces = [header]
    for r, g, b in pixels:
        r = min(255, max(0, int(r)))
        g = min(255, max(0, int(g)))
        b = min(255, max(0, int(b)))
        pieces.append(chr(r) + chr(g) + chr(b))
    command = ''.join(pieces)
    sock.send(command)


#-------------------------------------------------------------------------------
# Helper functions to make common color manipulations easier

def remap(x, oldmin, oldmax, newmin, newmax):
    """Remap the float x from the range oldmin-oldmax to the range newmin-newmax

    Does not clamp values that exceed min or max.
    For example, to make a sine wave that goes between 0 and 255:
        remap(math.sin(time.time()), -1, 1, 0, 256)

    """
    zero_to_one = (x-oldmin) / (oldmax-oldmin)
    return zero_to_one*(newmax-newmin) + newmin

def clamp(x, minn, maxx):
    """If float x is outside the range minn-maxx, return minn or maxx."""
    return max(minn, min(maxx, x))

def cos(x, offset=0, period=1, minn=0, maxx=1):
    """A cosine curve scaled to fit in a 0-1 range and 0-1 domain by default.

    offset: how much to slide the curve across the domain.
    period: the length of one wave
    minn, maxx: the output range
    """
    value = math.cos((x/period - offset) * math.pi * 2) / 2 + 0.5
    return value*(maxx-minn) + minn

def contrast(color, center, mult):
    """Expand the color values by a factor of mult around the pivot value of center.

    color: an (r, g, b) tuple
    center: a float -- the fixed point
    mult: a float -- expand or contract the values around the center point

    """
    r, g, b = color
    r = (r - center) * mult + center
    g = (g - center) * mult + center
    b = (b - center) * mult + center
    return (r, g, b)

def clip_black_by_luminance(color, threshold):
    """If the color's luminance is less than threshold, replace it with black.
    
    color: an (r, g, b) tuple
    threshold: a float

    """
    r, g, b = color
    if r+g+b < threshold*3:
        return (0, 0, 0)
    return (r, g, b)

def clip_black_by_channels(color, threshold):
    """Replace any r, g, or b value less than threshold with 0.

    color: an (r, g, b) tuple
    threshold: a float

    """
    r, g, b = color
    if r < threshold: r = 0
    if g < threshold: g = 0
    if b < threshold: b = 0
    return (r, g, b)

def mod_dist(a, b, n):
    """Return the distance between a and b, modulo n.

    a and b can be floats or integers.

    For example, thinking of a clock:
    mod_dist(11, 1, 12) == 2 because you can "wrap around".

    """
    return min((a-b) % n, (b-a) % n)

