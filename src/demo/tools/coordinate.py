#!/usr/bin/python

# coordinate
# tool to functionally dunp frames arriving at the demo's coordinator
# author: Christophe VG

import serial
import time

from xbee import ZigBee

ser  = serial.Serial("/dev/tty.usbserial-AD025LL3", 9600)
xbee = ZigBee(ser)

def handle(handlers={}):
  # continuously read and print packets
  while True:
    try:
      frame = xbee.wait_read_frame()

      # basic frame handling: unknown frames are dumped and skipped
      try:
        frame = {
          "rx"        : handle_rx,
        }[frame["id"]](frame)
      except KeyError:
        print "Unknown frame:", frame
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

  # extract from, hop, to info
  from_addr = _ba2str(frame["_rf_data"][0:2])
  hop_addr  = _ba2str(frame["_rf_data"][2:4])
  to_addr   = _ba2str(frame["_rf_data"][4:6])

  payload = frame["_rf_data"][6:]
  size = len(payload)

  # basic, single frames without headers
  # light reading   => size == 2
  # reputation info => size == 10
  # heartbeat       => size == 25
  
  # generated, (combined) frames with magic numbers
  # this can be a combined packet, parse it
  # headers:
  # heartbeat info:  0x00 0x01 + sequence(1) + time(4) + SHA1(20) => size = 27
  # reputation info: 0x00 0x02 + node(2) + a(4) + b(4)            => size = 12
  # reputation lost: 0x00 0x03                                    => size = 2

  try:
    {
      2 : _dump_light_or_generated_reputation_lost,
      10: _dump_reputation,
      12: _dump_generated_reputation,
      25: _dump_heartbeat,
      27: _dump_generated_heartbeat
    }[len(payload)](from_addr, hop_addr, to_addr, payload);
  except KeyError:
    try:
      _dump_combined_generated(from_addr, hop_addr, to_addr, payload)
    except:
      # print out frame in a "nice" format ;-)
      print "UNKNOWN payload"
      _dump("RX", {
        "length" : len(frame["_rf_data"]),
        "source" : _ba2str(frame["_source_addr_long"]) + " / " + \
                   _ba2str(frame["_source_addr"]),
        "options": ' '.join([opt for opt, val in frame["_options"].items() if val]),
        "data"   : _ba2str(frame["_rf_data"])
      })

  return frame

def _parse(data, config):
  return {key: data & byte == byte for key,byte in config.items()}

def _ba2str(bytearray):
  return ' '.join("%02x" % i for i in bytearray)

def _dump_light_or_generated_reputation_lost(from_addr, hop_addr, to_addr, payload):
  if payload[0] == 0x00 and payload[1] == 0x03:
    _dump_generated_reputation_lost(from_addr, hop_addr, to_addr)
  else:
    _dump_light(from_addr, hop_addr, to_addr, payload)

def _dump_light(from_addr, hop_addr, to_addr, payload):
  print "LIGHT           ", time.strftime("%H:%M:%S", time.gmtime()), \
        "from:", from_addr, (" " if to_addr == "00 00" else "B"),  \
        "=", round(((payload[0] << 8) + payload[1]) / 10.24, 2), "%"

def _dump_heartbeat(from_addr, hop_addr, to_addr, payload):
  # extract seq from payload
  seq = payload[0]
  print "HEARTBEAT       ", time.strftime("%H:%M:%S", time.gmtime()), \
        "from:", from_addr, (" " if to_addr == "00 00" else "B"),  \
        "=", "seq", seq

def _dump_reputation(from_addr, hop_addr, to_addr, payload):
  # extract node, alpha, beta
  node  = _ba2str(payload[0:2])
  alpha = _ba2str(payload[2:6])
  beta  = _ba2str(payload[6:10])
  print "REPUTATION      ", time.strftime("%H:%M:%S", time.gmtime()), \
        "from:", from_addr, (" " if to_addr == "00 00" else "B"),  \
        "=", "node", node, "alpha", alpha, "beta", beta

def _dump_generated_reputation_lost(from_addr, hop_addr, to_addr):
  print "REPUT LOST", time.strftime("%H:%M:%S", time.gmtime()), \
        "from:", from_addr, "offender:", to_addr

def _dump_generated_reputation(from_addr, hop_addr, to_addr, payload):
  _dump_reputation(from_addr, hop_addr, to_addr, payload[2:])
  
def _dump_generated_heartbeat(from_addr, hop_addr, to_addr, payload):
  _dump_heartbeat(from_addr, hop_addr, to_addr, payload[2:])

def _dump_combined_generated(from_addr, hop_addr, to_addr, payload):
  # heartbeat info:  0x00 0x01 + sequence(1) + time(4) + SHA1(20) => size = 27
  # reputation info: 0x00 0x02 + node(2) + a(4) + b(4)            => size = 12
  # reputation lost: 0x00 0x03                                    => size = 2
  lengths = { 0x01: 27, 0x02: 12, 0x03: 2 }
  index = 0
  print "COMBINED FRAME", "-" * 65
  while index < len(payload) and payload[index] == 0x00:
    # second part of magic number
    magic = payload[index+1]
    # if a KeyError occurs, parsing failed and the exception triggers handling
    # at highest level with raw packet info dump
    {
      0x01: _dump_generated_heartbeat,
      0x02: _dump_generated_reputation,
      0x03: _dump_generated_reputation_lost
    }[magic](from_addr, hop_addr, to_addr, payload[lengths[magic]:])
    # move index forward to next expected header
    index += lengths[magic]
  print "-" * 80

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

if __name__ == "__main__":
  handle()
