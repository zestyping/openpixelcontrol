#!/usr/bin/env python

from __future__ import division
import json
from math import cos, pi, sin
import optparse
import sys


#-------------------------------------------------------------------------------
# command line

parser = optparse.OptionParser(description='''
Creates a torus extending from min_degrees to max_degrees, where an angle
of 0 degrees is the +x axis and an angle of 90 degrees is the +z axis.
n_along is optional; it will default to a value that creates square pixels.
''')
parser.add_option('--major_radius', dest='major_radius', default=2,
                  action='store', type='float',
                  help='major radius of torus (default 2)')
parser.add_option('--minor_radius', dest='minor_radius', default=1,
                  action='store', type='float',
                  help='minor radius of torus (default 1)')
parser.add_option('--min_degrees', dest='min_degrees', default=0,
                  action='store', type='float',
                  help='starting angle of torus sector (default 0)')
parser.add_option('--max_degrees', dest='max_degrees',
                  action='store', type='float',
                  help='ending angle of torus sector')
parser.add_option('--n_around', dest='n_around', default=12,
                  action='store', type='int',
                  help='number of pixels around the torus (default 12)')
parser.add_option('--n_along', dest='n_along',
                  action='store', type='int',
                  help='number of pixels along the torus (optional)')
options, args = parser.parse_args()

# figure out how many pixels are needed along the torus to get square pixels
if not options.n_along:
    minor_circumference = options.minor_radius * 2 * pi
    sector_fraction = (options.max_degrees - options.min_degrees) / 360.0
    length = options.major_radius * 2 * pi * sector_fraction
    options.n_along = int(length * options.n_around / minor_circumference)

options.n_along = max(1, options.n_along)

class Vec:
    def __init__(self, x, y, z):
        self.x, self.y, self.z = x, y, z
    def __add__(self, v):
        return Vec(self.x + v.x, self.y + v.y, self.z + v.z)
    def __sub__(self, v):
        return Vec(self.x - v.x, self.y - v.y, self.z - v.z)
    def __mul__(self, k):
        return Vec(self.x * k, self.y * k, self.z * k)
    def __rmul__(self, k):
        return Vec(self.x * k, self.y * k, self.z * k)
    def __div__(self, k):
        return Vec(self.x / k, self.y / k, self.z / k)
    def __abs__(self):
        return (self.x*self.x + self.y*self.y + self.z*self.z)**0.5
    def __str__(self):
        return '[%.4f, %.4f, %.4f]' % (self.x, self.y, self.z)


#-------------------------------------------------------------------------------
# make torus

result = ['[']
angle_min = options.min_degrees * (2 * pi / 360)
if options.max_degrees and options.n_along > 1:
    angle_max = options.max_degrees * (2 * pi / 360)
    angle_inc = (angle_max - angle_min) / (options.n_along - 1)
else:
    angle_inc = (2 * pi) / options.n_along

for i in range(options.n_along):
    angle = angle_min + i*angle_inc
    lx = Vec(cos(angle), 0, sin(angle))
    ly = Vec(0, 1, 0)
    lz = Vec(-sin(angle), 0, cos(angle))

    for j in range(options.n_around):
        theta = j / options.n_around * 2 * pi
        r = lx * cos(theta) + ly * sin(theta)
        p = lx * options.major_radius + r * options.minor_radius
        result.append('  {"point": %s},' % p)

# trim off last comma
result[-1] = result[-1][:-1]

result.append(']')
print('\n'.join(result))

sys.stderr.write('\nn_around = %s\n' % options.n_around)
sys.stderr.write('n_along = %s\n' % options.n_along)
sys.stderr.write('total = %s\n\n' % (options.n_along*options.n_around))

