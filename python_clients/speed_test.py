#!/usr/bin/env python
"""A minimal test pattern.  Use this to measure your maximum frame rate."""

import optparse
import sys
import time

import opc_client


parser = optparse.OptionParser()
parser.add_option('-n', '--num_pixels', dest='num_pixels', default=100,
                    action='store', type='int', help='number of pixels')
parser.add_option('-s', '--server', dest='server', default='127.0.0.1:7890',
                    action='store', type='string', help='server IP and port')
parser.add_option('-f', '--fps', dest='fps', default=1e9,
                    action='store', type='int', help='frames per second')

options, args = parser.parse_args()
socket = opc_client.get_socket(options.server)

black_white = [(0, 0, 0), (2, 2, 2)]
rgb_bright = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
red_pixels = [(1, 0, 0)] * options.num_pixels
green_pixels = [(0, 1, 0)] * options.num_pixels
blue_pixels = [(0, 0, 1)] * options.num_pixels
arrays = [red_pixels, green_pixels, blue_pixels]
frame = 0
start_time, start_frame = time.time(), frame

print 'First pixel will flash on and off every 10 frames.'
print 'Second pixel will flash on and off every 100 frames.'
print 'Third pixel will flash on and off every 1000 frames.'
print
print 'Testing %d pixels (Ctrl-C to quit)...' % options.num_pixels

while True:
    for pixels, bright in zip(arrays, rgb_bright):
        dim = pixels[0]
        for i in range(options.num_pixels):
            pixels[i] = bright
            pixels[0] = black_white[(frame % 10)/5]
            pixels[1] = black_white[(frame % 100)/50]
            pixels[2] = black_white[(frame % 1000)/500]
            opc_client.put_pixels(socket, 0, pixels)
            pixels[i] = dim
            frame += 1
            if frame % 100 == 0:
                now = time.time()
                if now - start_time >= 1.0:
                    fps = (frame - start_frame) / (now - start_time)
                    sys.stdout.write('%7.1f fps\r' % fps)
                    sys.stdout.flush()
                    start_time, start_frame = now, frame
        pixels[0] = pixels[1] = pixels[2] = dim
