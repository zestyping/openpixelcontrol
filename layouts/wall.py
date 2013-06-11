#!/usr/bin/env python

spacing = 0.11  # m
lines = []
for c in range(-12, 13):
    rs = [range(50), reversed(range(50))][c % 2]
    for r in rs:
        lines.append('  {"point": [%.2f, %.2f, %.2f]}' %
                     (c*spacing, 0, (r - 24.5)*spacing))
print '[\n' + ',\n'.join(lines) + '\n]'
