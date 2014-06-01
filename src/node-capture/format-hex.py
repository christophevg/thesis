#!/opt/local/bin/python2.7

import fileinput
import string
import sys

for data in fileinput.input():
  line  = 0
  count = 0
  sys.stdout.write("%07x " % line)
  for value in data.split(","):
    num = int(value, 16)
    sys.stdout.write("%02x" % num)
    count += 1
    if count > 15:
      print
      line += 16
      sys.stdout.write("%07x " % line)
      count = 0
    else:
      sys.stdout.write(" ")
print
