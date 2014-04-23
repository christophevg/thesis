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
#include "reputation.h"

// configuration

#define METRICS_REPORT_INTERVAL 15000

// forward declarations
void init(void);
void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload);
void transmit(uint16_t from, uint16_t hop, uint16_t to,
              uint8_t size,  uint8_t* payload);
void report_metrics(void);

// our own address and that of our parent node
uint16_t address;
uint16_t parent;

void application_step(void) {
  static time_t next_measurement = 0;

  if( clock_get_millis() < next_measurement) { return; }
  next_measurement += 5000;

  avr_set_bit(STATUS_LED_PORT,    // turn on the green status led
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
    reputation_init();
  );

  while(TRUE) {
    application_step();
    
    xbee_receive();
    
    measure(heartbeat_step(););

    xbee_receive();

    measure(reputation_step(););

    xbee_receive();
    
    report_metrics();
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
  mesh_on_transmit(transmit);

  xbee_wait_for_association();    // wait until the network is available
  _log("xbee associated...\n");
  
  address = xbee_get_nw_address();
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
}

void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload)
{
  _log("received: ");
  printf("  source : %02x %02x ",  (uint8_t)(source >> 8), (uint8_t)source);
  printf("from : %02x %02x ",  (uint8_t)(from   >> 8), (uint8_t)from  );
  printf("hop : %02x %02x ",  (uint8_t)(hop    >> 8), (uint8_t)hop   );
  printf("to : %02x %02x ", (uint8_t)(to     >> 8), (uint8_t)to    );
  printf("size : %d\n", size);
  // printf("  payload : ");
  // for(uint8_t i=0; i<size; i++) {
  //   if(i && i % 10 == 0) { printf("\n            "); }
  //   printf("%02x ", payload[i]);
  // }
  // printf("\n");
  measure(
    heartbeat_receive(source, from, hop, to, size, payload);
    reputation_receive(source, from, hop, to, size, payload);
  );
}

void transmit(uint16_t from, uint16_t hop, uint16_t to,
              uint8_t size,  uint8_t* payload)
{
  measure(
    reputation_transmit(from, hop, to, size, payload);
  );
}

void report_metrics(void) {
  static time_t next_report = 0;
  static unsigned long total_frames  = 0,
                       total_bytes   = 0,
                       samples       = 0;
  if(next_report < clock_get_millis()) {
    xbee_metrics_t metrics = xbee_reset_counters();
    total_frames += metrics.frames;
    total_bytes  += metrics.bytes;
    samples++;
    _log("xbee: %d frames (avg:%u/tot:%lu) / %i bytes (avg:%u/tot:%lu)\n",
         metrics.frames, (unsigned int)(total_frames / samples), total_frames,
         metrics.bytes,  (unsigned int)(total_bytes  / samples), total_bytes);
    next_report += METRICS_REPORT_INTERVAL;
  }
}
