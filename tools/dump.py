#!/usr/bin/python

# a simple script to dump incoming packets

import serial
import time

from xbee import ZigBee

ser = serial.Serial('/dev/tty.usbserial-AD025LL3', 9600)

xbee = ZigBee(ser)

# Continuously read and print packets
while True:
  try:
    frame = xbee.wait_read_frame()

    # unknown frames are dumped and skipped
    if frame['id'] != "rx":
      print frame
      next

    # parse addresses and payload into bytearrays
    source  = map(ord, frame['source_addr_long'])
    source2 = map(ord, frame['source_addr'])
    data    = map(ord, frame['rf_data'])

    # parse out options into dict
    raw = map(ord, frame['options'])[0]
    options = {
      'acked'     : raw & 0x01 == 0x01,
      'broadcast' : raw & 0x02 == 0x02,
      'encrypted' : raw & 0x20 == 0x20,
      'end-device': raw & 0x40 == 0x40
    }
    
    # print out frame in a "nice" format ;-)
    print "-" * 80
    print "time    : ", time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime())
    print "length  : ", len(data)
    print "source  : ", ' '.join('%02x'%i for i in source), "/", \
                        ' '.join('%02x'%i for i in source2)
    print "options : ", ' '.join([key for key, value in options.items() if value])
    print "data    : ", ' '.join('%02x'%i for i in data)

    print "-" * 80

  except KeyboardInterrupt:
    break

ser.close()
