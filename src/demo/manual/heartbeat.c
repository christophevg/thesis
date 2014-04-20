// heartbeat.c
// a basic implementation of heartbeat-based neighbour availability monitoring
// author: Christophe VG

#include <string.h>
#include <stdio.h>

#include "../../sha1/sha1.h"

#include "moose/bool.h"
#include "moose/clock.h"

#include "../lib/log.h"
#include "../lib/network.h"

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

// our own cached address
uint16_t address;

void heartbeat_init(void) {
  address = xbee_get_nw_address();
}

void heartbeat_step(void) {
  time_t now = clock_get_millis();

  // send a heartbeat every 3 seconds
  if( now >= next_heartbeat) {
    heartbeat_payload_t payload = heartbeat_create_payload();
    _log("sending heartbeat : %02x %02x %02x %02x %02x\n",
         payload.data[0], payload.data[1], payload.data[2], payload.data[3], payload.data[4]);
    mesh_broadcast(address, HEARTBEAT_PAYLOAD_SIZE, payload.data);
    next_heartbeat = now + 3000;
  }
  
  // do background processing 1 seconds
  if( now >= next_processing) {
    heartbeat_process();
    next_processing = now + 1000;
  }
}

static void log_node(const char* msg, heartbeat_node_t node) {
  _log("%s : %02x %02x = seq: %d seen: %lu incidents: %d trust: %d\n",
       msg,
       (uint8_t)(node.address >> 8), (uint8_t)node.address,
       node.seq, node.seen, node.incidents, node.trust);
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

time_t extract_time(uint8_t *payload) {
  return (time_t)(payload[1]) << 24
       | (time_t)(payload[2]) << 16
       | (time_t)(payload[3]) << 8
       | (time_t)(payload[4]);
}

// processes payload received information
void heartbeat_receive(uint16_t from, uint8_t size, uint8_t *payload) {
  // skip our own heartbeat messages
  if(from == address) { return; }

  // quick check if packet might be a heartbeat packet, based on size
  // it MUST be equal to HEARTBEAT_PAYLOAD_SIZE
  if( size != HEARTBEAT_PAYLOAD_SIZE ) {
    return;
  }

  time_t           now  = clock_get_millis(),
                   time = extract_time(payload);
  heartbeat_node_t node = get_node(from);

  if(node.unknown) {
    node.address   = from;
    node.seq       = payload[0];
    node.seen      = now;
    node.time      = time;
    node.incidents = 0;
    node.unknown   = FALSE;
    node.trust     = TRUE;

    log_node("new node", node);
  }
  
  if(! node.trust) { return; }

  // verify payload -> signature
  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)payload, 5);
  
  if(sha1.result == shaSuccess &&
     memcmp(sha1.hash, &payload[5], SHA1HashSize) == 0)
  {
    node.seq  = payload[0];
    node.seen = now;
    node.time = time;

    log_node("receive heartbeat", node);
  } else {
    node.incidents++;
    _log("FAILED sha1 check\n");
  }
  
  update_node(node);
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
    if(nodes[i].trust) {
      if( now - nodes[i].seen > (10000 + 5000 * nodes[i].incidents)) {
        nodes[i].incidents++;
        log_node("late heartbeat", nodes[i]);
      }

      if( nodes[i].incidents >= 3 ) {
        nodes[i].trust = FALSE;
        log_node("trust lost", nodes[i]);
      }
    }
  }
}
