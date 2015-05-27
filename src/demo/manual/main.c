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
#include "../lib/application_step.c"
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

  // initialize receive interval
  time_t next_receive = clock_get_millis(); // start now
  
  while(TRUE) {
    // process incoming packets
    if( clock_get_millis() >= next_receive ) {
      xbee_receive();
      next_heartbeat += RECEIVE_INTERVAL;
    }

    // always do the application step
    application_step();
    
#ifdef WITH_HEARTBEAT
    measure(heartbeat_step(););
#endif

#ifdef WITH_REPUTATION
    measure(reputation_step(););
#endif
    
    report_metrics();
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
