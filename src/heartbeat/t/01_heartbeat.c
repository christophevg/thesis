// tests the heartbeat functionality
// author: Christophe VG

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wunused-function"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#include "../heartbeat.h"

// forward declarations
static void after(unsigned long);
static void payload_dump(heartbeat_payload_t);
static void validate_payload(uint8_t, unsigned long, heartbeat_payload_t);

int main(void) {
  heartbeat_payload_t payload;
  
  after(1000); payload = heartbeat_create_payload();
  validate_payload(1, 1000, payload);

  after(1000); payload = heartbeat_create_payload();
  validate_payload(2, 2000, payload);

  after(10000); payload = heartbeat_create_payload();
  validate_payload(3, 12000, payload);
  
  printf("RESULT: OK\n");
  
  exit(EXIT_SUCCESS);
}

// mocked implementation of functions used by heartbeat
static unsigned long millis = 0;
unsigned long clock_get_millis(void) {
  return millis;
}

// configuration params
static void after(unsigned long dt) {
  millis += dt;
}

// helper functions
static void payload_dump(heartbeat_payload_t payload) {
  for(int i=0; i<25; i++) {
    printf("0x%02X ", payload.data[i]);
  }
  printf("\n");
}

static void validate_payload(uint8_t sequence, unsigned long millis,
                             heartbeat_payload_t payload)
{
  uint8_t expected[25];

  expected[0] = sequence;
  expected[1] = millis >> 24;
  expected[2] = millis >> 16;
  expected[3] = millis >>  8;
  expected[4] = millis;

  SHA1(expected, 5, &expected[5]);
  
  assert(memcmp( payload.data, expected, 25 ) == 0);
}

#pragma clang diagnostic pop
