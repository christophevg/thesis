// main.c
// author: Christophe VG

// sets up serial and xbee communications and prints own network address and 
// that of its parent to the serial link.

#include <avr/io.h>

#include "moose/avr.h"
#include "moose/bool.h"
#include "moose/serial.h"
#include "moose/clock.h"
#include "moose/xbee.h"

// configuration of external components

#define STATUS_LED_PORT    PORTB  // PB0
#define STATUS_LED_PIN     0

// network config
#define DESTINATION XB_COORDINATOR

// forward declarations
void init(void);

int main(void) {
  init();

  uint16_t address = xbee_get_nw_address();
  uint16_t parent  = xbee_get_parent_address();
  
  printf("address : %02x %02x\n", (uint8_t)(address >> 8), (uint8_t)address);
  printf("parent  : %02x %02x\n", (uint8_t)(parent  >> 8), (uint8_t)parent );

  while(TRUE);
  return(0);
}

void init(void) {
  avr_init();                     // initialise MCU
  avr_set_bit(STATUS_LED_PORT,    // turn on the red status led
              STATUS_LED_PIN);

  clock_init();                   // init/start the millis clock

  serial_init();                  // initialize use of serial port(s)
  printf("\nserial link ready...\n");

  xbee_init();                    // initialize use of XBee module
  xbee_wait_for_association();    // wait until the network is available
  printf("\nxbee associated...\n");
}
