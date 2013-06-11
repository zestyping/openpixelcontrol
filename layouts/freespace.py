#!/usr/bin/env python

spacing = 0.11  # m
lines = []
for c in range(-12, 13):
    rs = [reversed(range(25)), range(25)][c % 2]
    for r in rs:
        x, y, z = -c*spacing, 0, (r - 12)*spacing
        if r < 7:
            z = (6.5 - 12)*spacing
            y = (r - 6.5)*spacing
        lines.append('  {"point": [%.2f, %.2f, %.2f]}' % (x, y, z))
print '[\n' + ',\n'.join(lines) + '\n]'
