  avr_init();                     // initialise MCU
  avr_adc_init();                 // initialize the ADC for normal readings
  
  clock_init();                   // init/start the millis clock

  serial_init();                  // initialize use of serial port(s)
  _log("serial link ready...\n");

  xbee_init();                    // initialize use of XBee module

  // setup virtual mesh
  xbee_on_receive(mesh_receive);

  xbee_wait_for_association();    // wait until the network is available
  _log("xbee associated...\n");
  
  address = xbee_get_nw_address();
#include "external.h"
#define NODES_T_H_name STR(NODES_T_H)
#ifdef NODES_T_H
  me      = nodes_lookup(address);
#endif
  parent  = xbee_get_parent_address();
  
  _log("my address : %02x %02x\n", (uint8_t)(address >> 8), (uint8_t)address);
  _log("my parent  : %02x %02x\n", (uint8_t)(parent  >> 8), (uint8_t)parent );

  mesh_init();
  
  avr_clear_bit(LIGHT_SENSOR_IO,  // make light sensor pin an input pin
                LIGHT_SENSOR_PIN);
                
  // if we're a router, give the end device time to get associated
  if(parent == XB_NW_ADDR_UNKNOWN) {
    _log("I'm a router, waiting for end-device to join\n");
    while(! mesh_child_connected() ) {
      _delay_ms(500L);
      xbee_receive();
    }
  }
