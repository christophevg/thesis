// heartbeat.c
// a basic implementation of heartbeat-based neighbour availability monitoring
// author: Christophe VG

#include <string.h>
#include <stdio.h>

#include "../../sha1/sha1.h"

#include "moose/bool.h"
#include "moose/clock.h"

#include "log.h"
#include "network.h"
#include "heartbeat.h"

// internal data
// a heartbeat sequence counter
static uint8_t heartbeat;

// keep last seen time for each node
static heartbeat_node_t nodes[HEARTBEAT_MAX_NODES];
static uint8_t          node_count = 0;
// a placeholder for unknown nodes
static heartbeat_node_t unknown = { .unknown = TRUE };

static unsigned long next_heartbeat  = 0,
                     next_processing = 0;

void heartbeat_step(void) {
  // send a heartbeat every 3 seconds
  if(clock_get_millis() >= next_heartbeat) {
    heartbeat_payload_t payload = heartbeat_create_payload();
    broadcast_bytes(payload.data, HEARTBEAT_PAYLOAD_SIZE);
    next_heartbeat += 3000;
  }
  
  // do background processing every second
  if(clock_get_millis() >= next_processing) {
    heartbeat_process();
    next_processing += 1000;
  }
}

static void dump_node(heartbeat_node_t node) {
  log("<%02x %02x> seq: %d seen: %lu trust: %d",
      (uint8_t)(node.address >> 8),
      (uint8_t)node.address,
      node.seq, node.seen, node.trust);
}

// NOTE this is a very basic implementation, we might need to implement it with
//      a tree or a hashtable based on the node's address
//      since the number of nodes is at most 3, this loop is as good ;-)
static heartbeat_node_t get_node(uint16_t address) {
  for(uint8_t i=0; i<HEARTBEAT_MAX_NODES; i++) {
    if(nodes[i].address == address) { return nodes[i]; }
  }
  return unknown;
}

static void add_node(heartbeat_node_t node) {
  if(node_count >= HEARTBEAT_MAX_NODES) { return; }
  nodes[node_count] = node;
  node_count++;
}

static void update_node(heartbeat_node_t node) {
  for(uint8_t i=0; i<HEARTBEAT_MAX_NODES; i++) {
    if(nodes[i].address == node.address) {
      nodes[i] = node;
      return;
    }
  }
  // unknown node ... add it
  add_node(node);
}

time_t make_time(uint8_t *payload) {
  return (time_t)(payload[1]) << 24
       | (time_t)(payload[2]) << 16
       | (time_t)(payload[3]) << 8
       | (time_t)(payload[4]);
}

// processes payload received information
void heartbeat_receive(uint16_t address, uint8_t size, uint8_t *payload) {
  // quick check if packet might be a heartbeat packet, based on size
  // it MUST be equal to HEARTBEAT_PAYLOAD_SIZE
  if( size != HEARTBEAT_PAYLOAD_SIZE ) { return; }

  time_t           now  = clock_get_millis(),
                   time = make_time(payload);
  heartbeat_node_t node = get_node(address);

  if(node.unknown) {
    node.address   = address;
    node.seq       = payload[0];
    node.seen      = now;
    node.time      = time;
    node.incidents = 0;
    node.unknown   = FALSE;

    printf("TRACKING NEW NODE: ");
    dump_node(node);
  }

  // verify payload -> signature
  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)&(payload), 5);

  if(sha1.result == shaSuccess &&
     memcmp(sha1.hash, &payload[5], SHA1HashSize == 0))
  {
    node.seq  = payload[0];
    node.seen = now;
    node.time = time;
  } else {
    node.incidents++;
  }
  
  update_node(node);
  printf("... received heartbeat "); dump_node(node);
}

// provides payload to send
heartbeat_payload_t heartbeat_create_payload(void) {
  heartbeat_payload_t payload;
  
  // increase the heartbeat sequence number
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
    memcpy(&payload.data[5], &sha1.hash, SHA1HashSize);
  }
  
  return payload;
}

// background processing: check if anything went wrong and do this in two steps
// check if node's heartbeat is late.
// after 3 incidents, mark the node as untrustworthy
void heartbeat_process(void) {
  time_t now = clock_get_millis();

  for(uint8_t i=0; i<node_count; i++) {
    if( now - nodes[i].seen > 10000 ) {
      nodes[i].incidents++;
      printf("!!! node's heartbeat is late. last seen=%lu / now=%lu\n",
             nodes[i].seen, now );
    }

    if( nodes[i].incidents >= 3 ) {
      nodes[i].trust = FALSE;
    }
  }
}
