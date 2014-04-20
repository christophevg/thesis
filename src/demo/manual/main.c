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

#include "../lib/timing.h"

#include "heartbeat.h"

// forward declarations
void init(void);
void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload);

// our own address and that of our parent node
uint16_t address;
uint16_t parent;

static unsigned long next_measurement = 0;

void application_step(void) {
  unsigned long now = clock_get_millis();
  if( now < next_measurement) { return; }
  next_measurement = now + 5000;

  avr_set_bit(STATUS_LED_PORT,      // turn on the green status led
              STATUS_LED_PIN);

  uint16_t reading;               // the 16-bit reading from the ADC
  uint8_t  values[2];             // the bytes containing the reading in bytes

  // read light sensor
  reading = avr_adc_read(LIGHT_SENSOR_PIN);
  values[0] = (reading >> 8);
  values[1] = reading;
  
  _log("light reading: %02x %02x\n", values[0], values[1]);

  // and send it to the coordinator through the mesh
  mesh_send(address, DESTINATION, 2, values);

  avr_clear_bit(STATUS_LED_PORT,  // turn off the green status led
                STATUS_LED_PIN);
}

int main(void) {
  init();
  
  measure(
    heartbeat_init();
  );

  while(TRUE) {
    application_step();
    
    xbee_receive();
    
    measure(
      heartbeat_step()
    );

    xbee_receive();
  }

  return(0);
}

void init(void) {
  avr_init();                     // initialise MCU
  avr_adc_init();                 // initialize the ADC for normal readings
  
  clock_init();                   // init/start the millis clock

  serial_init();                  // initialize use of serial port(s)
  _log("serial link ready...\n");

  xbee_init();                    // initialize use of XBee module

  // setup virtual mesh
  xbee_on_receive(mesh_receive);
  mesh_on_receive(receive);

  xbee_wait_for_association();    // wait until the network is available
  _log("xbee associated...\n");
  
  address = xbee_get_nw_address();
  parent  = xbee_get_parent_address();
  
  _log("my address : %02x %02x\n", (uint8_t)(address >> 8), (uint8_t)address);
  _log("my parent  : %02x %02x\n", (uint8_t)(parent  >> 8), (uint8_t)parent );

  mesh_init();
  
  avr_clear_bit(LIGHT_SENSOR_IO,  // make light sensor pin an input pin
                LIGHT_SENSOR_PIN);
}

void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload)
{
  // _log("received:\n");
  // printf("  source  : %02x %02x\n", (uint8_t)(source >> 8), (uint8_t)source);
  // printf("  from    : %02x %02x\n", (uint8_t)(from   >> 8), (uint8_t)from  );
  // printf("  hop     : %02x %02x\n", (uint8_t)(hop    >> 8), (uint8_t)hop   );
  // printf("  to      : %02x %02x\n", (uint8_t)(to     >> 8), (uint8_t)to    );
  // printf("  payload : ");
  // for(uint8_t i=0; i<size; i++) {
  //   if(i && i % 10 == 0) { printf("\n            "); }
  //   printf("%02x ", payload[i]);
  // }
  // printf("\n");
  measure(
    heartbeat_receive(from, size, payload);
  );
}
