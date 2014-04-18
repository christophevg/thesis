PLATFORMS_DIR=../../foo-lang/platforms

INCLUDE_PATH=$(PLATFORMS_DIR)

MOOSE_DIR=$(PLATFORMS_DIR)/moose

MORE_SRC = $(MOOSE_DIR)/avr.c    \
					 $(MOOSE_DIR)/clock.c  \
					 $(MOOSE_DIR)/sleep.c  \
					 $(MOOSE_DIR)/serial.c \
					 $(MOOSE_DIR)/xbee.c   \
					 ../lib/network.c
MORE_OBJECTS = $(patsubst %.c, %.o, $(MORE_SRC))

MCU=atmega1284p
F_CPU=8000000

AVRDUDE_PROGRAMMER=jtag2

# external hardware composition ... yummie :-)
MORE_CDEFS=-DSERIAL_ON_USART1 # -DXBEE_ON_USART0 == default

include $(MOOSE_DIR)/Makefile

clean: clean_avr clean_more_objects

clean_avr:
	@(cd $(MOOSE_DIR); $(MAKE) clean)

clean_more_objects:
	@rm -f $(MORE_OBJECTS)	../lib/network.lst

# setup and control targets

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m

serial-digitus:
	screen /dev/tty.usbserial-FTSJ84AI 9600

serial-logilink:
	screen /dev/tty.usbserial 9600
