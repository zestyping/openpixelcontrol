#!/usr/bin/env python

from __future__ import division
import math
import optparse
import sys


#-------------------------------------------------------------------------------
# command line

parser = optparse.OptionParser(description="""Creates a wall
on the z axis extending from -height/2 to height/2 and along
the x axis from -width/2 to width/2.
n_tall is optional -- it will default to a value that
creates square pixels.  You can also create lines by setting
height to 0.
""")
parser.add_option('--height', dest='height', default=1,
                    action='store', type='float',
                    help='height of wall.  default = 1')
parser.add_option('--width', dest='width', default=1,
                    action='store', type='float',
                    help='width of wall.  default = 1')
parser.add_option('--n_wide', dest='n_wide', default=32,
                    action='store', type='int',
                    help='number of pixels along the x axis.  default = 32')
parser.add_option('--n_tall', dest='n_tall',
                    action='store', type='int',
                    help='number of pixels from top to bottom. (optional)')
options, args = parser.parse_args()

# figure out how many pixels are needed around the cylinder
# in order to get square pixels
if not options.n_tall:
    options.n_tall = int()
    options.n_tall = int(options.height/options.width * options.n_wide)

options.n_tall = max(1, options.n_tall)

#print "width {}, height {}, wide {}, tall {}".format(options.width, options.height,
#    options.n_wide, options.n_tall)
#exit()

#-------------------------------------------------------------------------------
# make cylinder

result = ['[']
for ii in range(options.n_tall):
    z = (-0.5 + 1/options.n_tall * (0.5 + ii)) * options.height

    for jj in range(options.n_wide):
        x = (-0.5 + 1/options.n_wide * (0.5 + jj)) * options.width;
#        theta = jj / options.n_around * math.pi * 2
#        x = math.sin(theta) * options.radius
#        y = math.cos(theta) * options.radius

        result.append('  {"point": [%.4f, %.4f, %.4f]},' % (x, 0, z))

# trim off last comma
result[-1] = result[-1][:-1]

result.append(']')
print '\n'.join(result)

sys.stderr.write('\nn_wide = %s\n' % options.n_wide)
sys.stderr.write('n_tall = %s\n' % options.n_tall)
sys.stderr.write('total = %s\n\n' % (options.n_tall*options.n_wide))
