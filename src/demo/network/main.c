// main.c
// author: Christophe VG

// implements 

#include <avr/io.h>

#include "moose/avr.h"
#include "moose/bool.h"
#include "moose/serial.h"
#include "moose/clock.h"
#include "moose/xbee.h"

#include "../lib/hardware.h"
#include "../lib/log.h"
#include "../lib/network.h"

// forward declarations
void init(void);
void handle_payload(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
                    uint8_t size,  uint8_t* payload);

uint16_t address, parent;

int main(void) {
  init();

  // send a packet to our destination (coordinator)
  uint8_t msg1[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
  _log("sending single message to coordinator\n");
  mesh_send(address, DESTINATION, 5, msg1);
  
  uint8_t msg2[5] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
  _log("broadcasting single message\n");
  mesh_broadcast(address, 5, msg2);

  while(TRUE) {
    xbee_receive();
  }
  return(0);
}

void init(void) {
  avr_init();                     // initialise MCU
  avr_set_bit(STATUS_LED_PORT,    // turn on the red status led
              STATUS_LED_PIN);

  clock_init();                   // init/start the millis clock

  serial_init();                  // initialize use of serial port(s)
  _log("serial link ready...\n");

  xbee_init();                    // initialize use of XBee module

  // setup virtual mesh
  xbee_on_receive(mesh_receive);
  mesh_on_receive(handle_payload);

  xbee_wait_for_association();    // wait until the network is available
  _log("xbee associated...\n");

  address = xbee_get_nw_address();
  parent  = xbee_get_parent_address();

  _log("my address : %02x %02x\n", (uint8_t)(address >> 8), (uint8_t)address);
  _log("my parent  : %02x %02x\n", (uint8_t)(parent  >> 8), (uint8_t)parent );

  mesh_init();

  // if we're a router, give the end device time to get associated
  if(parent == XB_NW_ADDR_UNKNOWN) {
    _log("I'm a router, waiting for end-device to join\n");
    while(! mesh_child_connected() ) {
      _delay_ms(500L);
      xbee_receive();
    }
  }
}

void handle_payload(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
                    uint8_t size,  uint8_t* payload)
{
  _log("received:\n");
  printf("  source  : %02x %02x\n", (uint8_t)(source >> 8), (uint8_t)source);
  printf("  from    : %02x %02x\n", (uint8_t)(from   >> 8), (uint8_t)from  );
  printf("  hop     : %02x %02x\n", (uint8_t)(hop    >> 8), (uint8_t)hop   );
  printf("  to      : %02x %02x\n", (uint8_t)(to     >> 8), (uint8_t)to    );
  printf("  payload : ");
  for(uint8_t i=0; i<size; i++) {
    if(i && i % 10 == 0) { printf("\n            "); }
    printf("%02x ", payload[i]);
  }
  printf("\n");
}
