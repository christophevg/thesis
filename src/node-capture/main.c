// This is a simple program that augments a counter and prints this value to a
// terminal via its USART port.
//
// author: Christophe VG

#include <avr/io.h>
#include <util/delay.h>

#include "../avr/avr.h"
#include "../avr/serial.h"    // wires printf to the USART port

unsigned int counter = 0;

int main(void) {
  avr_init();
  serial_init();

  while(1) {
    counter++;
    printf("counter = %i\n", counter);
    _delay_ms(1000);
  }

  return(0);
}
