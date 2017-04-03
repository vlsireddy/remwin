#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

while True:
    try:
	   print "DELL"
       s = sys.stdin.readline()
       sys.stdout.write(s)
       sys.stdout.flush()
    except EOFError, KeyboardInterrupt:
        break