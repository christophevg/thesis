#!/bin/bash

# automated script to show how node capture (in this case target=memory) can be
# performed
# author: Christophe VG

JTAG_USB_PORT=usb:5a:cb

echo "*** starting avarice..."
avarice --detach --mkII --capture --jtag $JTAG_USB_PORT :4242 > /dev/null

echo "*** performing memory dump using gdb..."
avr-gdb --batch -x dump-data-mem.gdb > /dev/null
hexdump dump.bin > dump1.hex

echo "*** waiting for second iteration..."
sleep 5

echo "*** starting avarice..."
avarice --detach --mkII --capture --jtag $JTAG_USB_PORT :4242 > /dev/null

echo "*** performing memory dump using gdb..."
avr-gdb --batch -x dump-data-mem.gdb > /dev/null
hexdump dump.bin > dump2.hex

echo "*** difference between dumps..."
diff dump1.hex dump2.hex

# clean up ;-)
rm -f dump.bin dump1.hex dump2.hex
