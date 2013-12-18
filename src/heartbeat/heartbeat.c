// heartbeat.c
// author: Christophe VG

#include <string.h>
#include <stdio.h>

#include "../sha1/sha1.h"

#include "../an-avr-lib/clock.h"
#include "heartbeat.h"

// a basic implementation of heartbeat-based neighbour availability monitoring

// internal data
// a heartbeat sequence counter
static uint8_t heartbeat;

// keep last seen time for each node
static heartbeat_node_t nodes[HEARTBEAT_MAX_NODES];
static uint8_t node_count = 0;
static heartbeat_node_t unknown = { .flags = HEARTBEAT_NODE_UNKNOWN };


static void dump_node(heartbeat_node_t node) {
  printf("<%02x %02x> seq: %d seen: %lu time: %lu diff: %li sus: %i inv: %i\n",
         (uint8_t)(node.address >> 8),
         (uint8_t)node.address,
         node.seq, node.seen, node.time, node.diff,
         (node.flags & HEARTBEAT_NODE_SUSPICIOUS) > 0,
         (node.flags & HEARTBEAT_NODE_INVALID_SIG) > 0);
}

// TODO this is a very basic implementation, we might need to implement it with
//      a tree or a hashtable based on the node's address
static heartbeat_node_t get_node(uint16_t address) {
  for(uint8_t i=0; i<HEARTBEAT_MAX_NODES; i++) {
    if(nodes[i].address == address) { return nodes[i]; }
  }
  return unknown;
}

static void add_node(heartbeat_node_t node) {
  if(node_count < HEARTBEAT_MAX_NODES) {
    nodes[node_count] = node;
    node_count++;
  } else {
    // whoops ;-)
  }
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
  // TODO introduce some form of magic cookie ? BEAT = 8347 ?
  if( size != HEARTBEAT_PAYLOAD_SIZE ) { return; }

  time_t           now  = clock_get_millis(),
                   time = make_time(payload);
  long             diff = time - now;
  heartbeat_node_t node = get_node(address);

  if(node.flags & HEARTBEAT_NODE_UNKNOWN) {
    // unknown node? -> initialize it
    node.address = address;
    node.seq     = payload[0];
    node.seen    = now;
    node.time    = time;
    node.diff    = diff;
    node.flags   = HEARTBEAT_NODE_OK;
    printf("TRACKING NEW NODE: "); dump_node(node);
  }

  // verify payload -> signature
  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)&(payload), 5);
  if(sha1.result == shaSuccess) {
    // this is a valid timestamp (well at least 4 bytes)
    node.flags &= ~HEARTBEAT_NODE_INVALID_SIG;

    // TODO: check for diff changes -> set flag ?

    // update statistics
    if(node.seq + 1 != payload[0]) {
      printf(">>> sequence skipped\n");
      node.flags |= HEARTBEAT_NODE_SEQ_SKIP;
    } else {
      node.flags &= ~HEARTBEAT_NODE_SEQ_SKIP;
    }
    node.seq  = payload[0];
    node.seen = now;
    node.time = time;
    node.diff = diff;

    update_node(node);
    printf("... received heartbeat "); dump_node(node);

    return;
  }
  
  // not a valid payload.

  printf("!!! INVALID SIGNATURE\n");

  // two possibilities: 1) not a heartbeat, 2) invalid signature
  // let's try to eliminate the first: if the first 4 bytes of the payload are
  // a timestamp and the signature is wrong, then they timestamp should be close
  // to the one we expect from a known node
  long local_time_diff = time - diff - now;
  if(local_time_diff < 1000 || local_time_diff > -1000) {
    // node's time is within range of 1 second of ours, so it's a heartbeat
    // with an invalid signature, mark it as such and update its information
    node.seen   = now;
    node.time   = time;
    node.diff   = (node.diff + diff) / 2;
    node.flags |= HEARTBEAT_NODE_INVALID_SIG;
    update_node(node);
    return;
  }
  
  printf("===> DROPPED NO HEARTBEAT\n");
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
  
  // TODO: a unique shared secret or something should be added here, but hey,
  //       it's a demo ;-)
  
  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)&(payload.data), 5);
  if(sha1.result == shaSuccess) {
    // TODO: check impact of linking string.h for this memcpy
    //       alternative: small for-loop
    memcpy(&payload.data[5], &sha1.hash, SHA1HashSize);
  }
  
  return payload;
}

// background processing: check if anything went wrong and do this in two steps
// 1. check if node's heartbeat is late
//                    last heartbeat's signature was invalid
// 2. if any of these conditions hold, mark node as suspicious and reset flags
// 3. if any of these conditions hold AND the node is suspicious -> ALERT
void heartbeat_process(void) {
  time_t now = clock_get_millis();

  for(uint8_t i=0; i<node_count; i++) {
    uint8_t ok = 1;
    if( now - nodes[i].seen > 10000 ) {
      printf("!!! node's heartbeat is late. last seen=%lu / now=%lu\n",
             nodes[i].seen, now );
      ok = 0;
    } else if( nodes[i].flags & HEARTBEAT_NODE_INVALID_SIG ) {
      printf("!!! node has invalid signature\n");
      ok = 0;
    }

    if( !ok ) {
      if(nodes[i].flags & HEARTBEAT_NODE_SUSPICIOUS) {
        // the node was already suspicious -> ALERT !!!
        printf("!!! ALERT !!! for ");
      } else {
        printf("... warning ... for ");
        nodes[i].flags |=  HEARTBEAT_NODE_SUSPICIOUS;
        nodes[i].flags &= ~HEARTBEAT_NODE_INVALID_SIG; // clear it once
      }
      dump_node(nodes[i]);
    }
  }
}
