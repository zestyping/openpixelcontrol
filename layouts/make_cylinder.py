#!/usr/bin/env python

from __future__ import division
import math
import optparse
import sys


#-------------------------------------------------------------------------------
# command line

parser = optparse.OptionParser(description="""Creates a cylinder
on the z axis extending from -height/2 to height/2.
n_tall is optional -- it will default to a value that
creates square pixels.  You can also create circles by setting
height to 0.
""")
parser.add_option('--radius', dest='radius', default=1,
                    action='store', type='float',
                    help='radius of cylinder. default = 1')
parser.add_option('--height', dest='height', default=1,
                    action='store', type='float',
                    help='height of cylinder.  default = 1')
parser.add_option('--n_around', dest='n_around', default=32,
                    action='store', type='int',
                    help='number of pixels around the circumference.  default = 32')
parser.add_option('--n_tall', dest='n_tall',
                    action='store', type='int',
                    help='number of pixels from top to bottom. (optional)')
options, args = parser.parse_args()

# figure out how many pixels are needed around the cylinder
# in order to get square pixels
if not options.n_tall:
    options.n_tall = int(options.n_around * options.height / options.radius / math.pi)

options.n_tall = max(1, options.n_tall)

#-------------------------------------------------------------------------------
# make cylinder

result = ['[']
for ii in range(options.n_tall):
    if options.n_tall == 1:
        z = 0
    else:
        z = (ii / (options.n_tall-1)) * 2 - 1
    z *= options.height

    for jj in range(options.n_around):
        theta = jj / options.n_around * math.pi * 2
        x = math.sin(theta) * options.radius
        y = math.cos(theta) * options.radius

        result.append('  {"point": [%.4f, %.4f, %.4f]},' % (x, y, z))

# trim off last comma
result[-1] = result[-1][:-1]

result.append(']')
print '\n'.join(result)

sys.stderr.write('\nn_around = %s\n' % options.n_around)
sys.stderr.write('n_tall = %s\n' % options.n_tall)
sys.stderr.write('total = %s\n\n' % (options.n_tall*options.n_around))

