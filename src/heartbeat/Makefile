MORE_SRC = ../an-avr-lib/avr.c    ../an-avr-lib/clock.c ../an-avr-lib/sleep.c  \
					 ../an-avr-lib/serial.c ../an-avr-lib/xbee.c \
					 heartbeat.c ../sha1/sha1.c
MORE_OBJECTS = $(patsubst %.c, %.o, $(MORE_SRC))

MCU=atmega1284p
F_CPU=8000000

AVRDUDE_PROGRAMMER=jtag2

# external hardware composition ... yummie :-)
MORE_CDEFS=-DSERIAL_ON_USART1 # -DXBEE_ON_USART0 == default

#MORE_CDEFS=-DDEBUG -DSERIAL_ON_USART1 # -DXBEE_ON_USART0 == default

include ../an-avr-lib/Makefile

clean: clean_avr clean_more_objects

clean_avr:
	@(cd ../an-avr-lib; $(MAKE) clean)

clean_more_objects:
	@rm -f $(MORE_OBJECTS)

# setup and control targets

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m

serial-digitus:
	screen /dev/tty.usbserial-FTSJ84AI 9600

serial-logilink:
	screen /dev/tty.usbserial 9600
