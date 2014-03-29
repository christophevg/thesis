# xbee_handler
# basic wrapper module for standard XBee module
# author: Christophe VG

import serial
import time

from xbee import ZigBee

ser  = serial.Serial("/dev/tty.usbserial-AD025LL3", 9600)
xbee = ZigBee(ser)

def send(cmd, **kwargs):
  xbee.send(cmd, **kwargs)

def handle(handlers={}):
  # continuously read and print packets
  while True:
    try:
      frame = xbee.wait_read_frame()

      # basic frame handling: unknown frames are dumped and skipped
      try:
        frame = {
          "rx"        : handle_rx,
          "tx_status" : handle_tx_status
        }[frame["id"]](frame)
      except KeyError:
        print "Unknown frame:", frame
        continue

      # apply handlers
      try:
        handlers[frame["id"]](frame)
      except KeyError:
        continue

    except KeyboardInterrupt: break
  ser.close()

def handle_rx(frame):
  # convert all bytestrings into bytearrays
  frame = dict(frame.items() + {'_'+key:map(ord, value) \
    for key, value in frame.items()}.items())

  # parse out options into dict
  frame["_options"] = _parse(frame["_options"][0], {
    "acked"     : 0x01,
    "broadcast" : 0x02,
    "encrypted" : 0x20,
    "end-device": 0x40
  })
  
  # print out frame in a "nice" format ;-)
  _dump("RX", {
    "length" : len(frame["_rf_data"]),
    "source" : _ba2str(frame["_source_addr_long"]) + " / " + \
               _ba2str(frame["_source_addr"]),
    "options": ' '.join([opt for opt, val in frame["_options"].items() if val]),
    "data"   : _ba2str(frame["_rf_data"])
  })

  return frame

def handle_tx_status(frame):
  # convert all bytestrings into bytearrays
  frame = dict(frame.items() + {'_'+key:map(ord, value) \
    for key, value in frame.items()}.items())

  # parse out deliver status bytes into dicts
  frame["_deliver_status"] = _parse(frame["_deliver_status"][0], {
    "success"     : 0x00,
    "mac"         : 0x01,
    "cca"         : 0x02,
    "inv.dest"    : 0x15,
    "nw.ack"      : 0x21,
    "not-joined"  : 0x22,
    "self-addr."  : 0x23,
    "address?"    : 0x24,
    "route?"      : 0x25,
    "neighbour?"  : 0x26,
    "inv.binding" : 0x2b,
    "resource!"   : 0x2c,
    "brdcst+aps"  : 0x2d,
    "uni+aps+ee=0": 0x2e,
    "resource!bis": 0x32,
    "too-big"     : 0x74,
    "indirect"    : 0x75
  })

  # parse out discover status bytes into dicts
  frame["_discover_status"] = _parse(frame["_discover_status"][0], {
    "no"      : 0x00,
    "address" : 0x01,
    "route"   : 0x02,
    "extended": 0x40
  })
  
  _dump("TX STATUS", {
    "frame"       : _ba2str(frame["_frame_id"]),
    "destination" : _ba2str(frame["_dest_addr"]),
    "retries"     : str(frame["_retries"][0]),
    "delivery"    : ' '.join([key for key, value in \
                                frame["_deliver_status"].items() if value]),
    "discovery"   : ' '.join([key for key, value in \
                                frame["_discover_status"].items() if value])
  })

  return frame

def _parse(data, config):
  return {key: data & byte == byte for key,byte in config.items()}

def _ba2str(bytearray):
  return ' '.join("%02x" % i for i in bytearray)

def _dump(type, data):
  # determine max length of a label
  pad = max(11, max([len(label) for label in data]))

  # pretty print the data
  print "-" * 80
  print type
  print "-" * 80
  # add a local timestamp
  print "time", " " * (pad-4), ":", time.strftime("%a, %d %b %Y %H:%M:%S +0000",
                                                  time.gmtime())
  for label, value in data.items():
    print label, " " * (pad-len(label)), ":", str(value)
  print "-" * 80
