#!/usr/bin/python

# a simple script to echo back any received packet

from xbee_handler import handle, send

def echo(frame):
  if frame["id"] != "rx": return
  # echo back the packet
  destination_long =  "\x00\x00\x00\x00\x00\x00\xFF\xFF" \
    if frame["_options"]["broadcast"] else frame["source_addr_long"]

  destination =  "\xFF\xFE" if frame["_options"]["broadcast"] \
  else frame["source_addr"]

  send("tx",
       dest_addr_long=destination_long,
       dest_addr=destination,
       data=frame["rf_data"])

if __name__ == "__main__":
  handle({"rx": echo})
