#!/usr/bin/env python

import json
import sys

result = []
for i in range(232):
    result.append({'point': [(i + 1)/32.0, 0, 0]})
    result.append({'point': [-(i + 1)/32.0, 0, 0]})

json.dump(result, sys.stdout, indent=2)
