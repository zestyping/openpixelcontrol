#!/usr/bin/env python

spacing = 0.11  # m
lines = []
for c in range(0, 10):
    for r in range(0, 20):
        lines.append('  {"point": [%.2f, %.2f, %.2f]}' % (c * spacing, 0, r * spacing))
print '[\n' + ',\n'.join(lines) + '\n]'
