// main.c
// author: Christophe VG

// implements 

#include <avr/io.h>

#include "moose/avr.h"
#include "moose/bool.h"
#include "moose/serial.h"
#include "moose/clock.h"
#include "moose/xbee.h"

#include "../lib/application.h"

#include "../lib/config.h"

#ifdef WITH_HEARTBEAT
#include "heartbeat.h"
#endif

#ifdef WITH_REPUTATION
#include "reputation.h"
#endif

// forward declarations
void init(void);
void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload);
void transmit(uint16_t from, uint16_t hop, uint16_t to,
              uint8_t size,  uint8_t* payload);

void application_step(void) {
  uint16_t reading;               // the 16-bit reading from the ADC
  uint8_t  values[2];             // the bytes containing the reading in bytes

  // read light sensor
  reading = avr_adc_read(LIGHT_SENSOR_PIN);
  values[0] = (reading >> 8);
  values[1] = reading;
  
  _log("light reading: %02x %02x\n", values[0], values[1]);

  // and send it to the coordinator through the mesh
  mesh_send(address, DESTINATION, 2, values);
}

int main(void) {
  init();

#if defined(WITH_HEARTBEAT) || defined(WITH_REPUTATION)
  measure(
#ifdef WITH_HEARTBEAT
    heartbeat_init();
#endif
#ifdef WITH_REPUTATION
    reputation_init();
#endif
  );
#endif

  time_t now = clock_get_millis();

  // initialize receive interval
  time_t next_receive     = now;
  time_t next_report      = now + METRICS_REPORT_INTERVAL;
  time_t next_heartbeat   = now;
  time_t next_processing  = now + PROCESSING_INTERVAL;
  time_t next_validation  = now + VALIDATION_INTERVAL;
  time_t next_sharing     = now + SHARING_INTERVAL;
  time_t next_measurement = now;  

  while(TRUE) {
    now = clock_get_millis();
    cycles++;
    
    // process incoming packets
    if( now >= next_receive ) {
      xbee_receive();
      next_receive += RECEIVE_INTERVAL;
      continue;
    }

    // the application step
    if( now >= next_measurement) {
      application_step();
      next_measurement += 5000;
      continue;
    }
    
#ifdef WITH_HEARTBEAT
    // send a heartbeat
    if( now >= next_heartbeat ) {
      heartbeat_send();
      next_heartbeat += HEARTBEAT_INTERVAL;
      continue;
    }
    if( now >= next_processing ) {
      heartbeat_process();
      next_processing += PROCESSING_INTERVAL;
      continue;
    }
#endif

#ifdef WITH_REPUTATION
    // validate known nodes
    if( now >= next_validation ) {
        reputation_validate();
        next_validation += VALIDATION_INTERVAL;
        continue;
    }
  
    // share reputation information
    if( now >= next_sharing ) {
      reputation_share();
      next_sharing += SHARING_INTERVAL;
      continue;
    }
#endif
    
    // reporting
    if( now >= next_report ) {
      report_metrics();
      next_report += METRICS_REPORT_INTERVAL;
      continue;
    }
  }

  return(0);
}

void init(void) {
#include "../lib/init.c"
  // setup handlers
#if defined(WITH_HEARTBEAT) || defined(WITH_REPUTATION)
  mesh_on_receive(receive);
  mesh_on_transmit(transmit);
#endif
}

void receive(uint16_t source, uint16_t from, uint16_t hop, uint16_t to,
             uint8_t size,  uint8_t* payload)
{
#if defined(WITH_HEARTBEAT) || defined(WITH_REPUTATION)
  measure(
#ifdef WITH_HEARTBEAT
    heartbeat_receive(source, from, hop, to, size, payload);
#endif
#ifdef WITH_REPUTATION
    reputation_receive(source, from, hop, to, size, payload);
#endif
  );
#endif
}

void transmit(uint16_t from, uint16_t hop, uint16_t to,
              uint8_t size,  uint8_t* payload)
{
#ifdef WITH_REPUTATION
  measure(
    reputation_transmit(from, hop, to, size, payload);
  );
#endif
}
