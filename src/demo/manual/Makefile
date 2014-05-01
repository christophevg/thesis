HEARTBEAT   = yes
REPUTATION  = yes
FAILURE_PCT = 0

PLATFORMS_DIR=../../foo-lang/platforms

INCLUDE_PATH=$(PLATFORMS_DIR)

MOOSE_DIR=$(PLATFORMS_DIR)/moose

MORE_SRC = $(wildcard $(MOOSE_DIR)/*.c) \
					 ../lib/network.c

ifeq ($(HEARTBEAT), yes)
MORE_SRC   += heartbeat.c	 ../../sha1/sha1.c
MORE_CDEFS += -DWITH_HEARTBEAT
endif

ifeq ($(REPUTATION), yes)
MORE_SRC   += reputation.c
MORE_CDEFS += -DWITH_REPUTATION
endif

MORE_CDEFS += -DFORWARD_FAILURE_PCT=$(FAILURE_PCT)

MORE_OBJECTS = $(patsubst %.c, %.o, $(MORE_SRC))

MCU=atmega1284p
F_CPU=8000000

AVRDUDE_PROGRAMMER=jtag2

# external hardware composition ... yummie :-)
MORE_CDEFS+=-DSERIAL_ON_USART1 # -DXBEE_ON_USART0 == default
# MORE_CDEFS+=-DDEBUG

include $(MOOSE_DIR)/Makefile

clean: clean_avr clean_more_objects

clean_avr:
	@(cd $(MOOSE_DIR); $(MAKE) clean)

clean_more_objects:
	@rm -f $(MORE_OBJECTS)

# setup and control targets

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m

serial-digitus:
	screen /dev/tty.usbserial-FTSJ84AI 9600

serial-logilink:
	screen /dev/tty.usbserial 9600
