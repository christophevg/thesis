// heartbeat.c
// author: Christophe VG

#include <string.h>

#include "../sha1/sha1.h"

#include "../an-avr-lib/clock.h"
#include "heartbeat.h"

// a basic implementation of heartbeat-based neighbour availability monitoring

// internal data
// a heartbeat sequence counter
static uint8_t heartbeat;
// keep last seen time for each node
// static unsigned long nodes[HEARTBEAT_MAX_NODES];

// processes payload received information
void heartbeat_receive(uint16_t source, uint8_t size, uint8_t *payload) {
  // TODO
}

// provides payload to send
heartbeat_payload_t heartbeat_create_payload(void) {
  heartbeat_payload_t payload;
  
  // up the heartbeat sequence number
  payload.data[0] = ++heartbeat;

  // get the time
  uint32_t millis = clock_get_millis();
  payload.data[1] = millis >> 24;
  payload.data[2] = millis >> 16;
  payload.data[3] = millis >>  8;
  payload.data[4] = millis;
  
  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)&(payload.data), 5);
  if(sha1.result == shaSuccess) {
    // TODO: check impact of linking string.h for this memcpy
    //       alternative: small for-loop
    memcpy(&payload.data[5], &sha1.hash, SHA1HashSize);
  }
  
  return payload;
}

// background processing
void heartbeat_process(void) {
  // TODO
}
