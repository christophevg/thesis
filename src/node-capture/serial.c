// serial.c
// author: Christophe VG <contact@christophe.vg>

// functions to operate the UART on AVR/ATMega

#include <stdio.h>
#include <avr/io.h>

#include "avr.h"
#include "serial.h"

// trick to make printf "print" to serial<->UART
static FILE mystdout = FDEV_SETUP_STREAM(serial_putchar, NULL, _FDEV_SETUP_WRITE);

void serial_init(void) {
  // make MISO pin input pin by clearing it
  avr_clear_bit(MISO_PORT, MISO_PIN);
  // make RX pin input pin by clearing it
  avr_clear_bit(RX_PORT, RX_PIN);

  // set USART Baud rate
  UBRR1H = MYUBRR >> 8;
  UBRR1L = MYUBRR;
  // enable receiver and transmitter
  UCSR1B = (1 << RXEN1) | (1 << TXEN1);
    
  stdout = &mystdout; // required for printf init
}

static int serial_putchar(char c, FILE *stream) {
  if (c == '\n') serial_putchar('\r', stream); // add a CR before the LF

  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;

  return 0;
}

uint8_t serial_getchar(void) {
  while( !(UCSR1A & (1<<RXC1)) );
  return(UDR1);
}
