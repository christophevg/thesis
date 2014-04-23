// heartbeat.c
// a basic implementation of heartbeat-based neighbour availability monitoring
// author: Christophe VG

#include <string.h>

#include "moose/bool.h"
#include "moose/clock.h"
#include "moose/xbee.h"

#include "../../sha1/sha1.h"

#include "../lib/log.h"
#include "../lib/network.h"

#include "heartbeat.h"

// configuration

#define PROCESSING_INTERVAL 1000  // process every 1s
#define HEARTBEAT_INTERVAL  3000  // send out a heartbeat every 3s
#define MAX_INCIDENTS          3  // number of incidents before trust is gone

#define MAX_NODES              5  // maximum number of tracked neighbours
// payload consists of  1 byte for the heartbeat sequence
//                 and  4 bytes for the node's time in millis
//                 and 20 bytes for the SHA1 hash of sequence + millis
//                   = 25 bytes payload
#define PAYLOAD_SIZE          25  // allows for constant length arrays

// private type

// struct to keep track of nodes' last seen time and status
typedef struct {
  uint16_t address;    // the network address of the node
  uint8_t  seq;        // last sequence id seen
  time_t   seen;       // the time when we saw the node (our time)
  uint8_t  incidents;  // counter for incidents
  bool     trust;      // to trust or not to trust, that is the question
} heartbeat_node_t;

// internal data

// a heartbeat sequence counter
static uint8_t heartbeat;

// keep last seen time for each node
static heartbeat_node_t nodes[MAX_NODES];
static uint8_t          node_count = 0;

// intervals markers
static time_t next_heartbeat  = 0,
              next_processing = 0;

// our own cached address
uint16_t me;

// forward declarations of private helper functions
static void              _beat(void);
static void              _process(void);
static heartbeat_node_t* _get_node(uint16_t address);
static void              _log_node(const char* msg, heartbeat_node_t* node);

// public interface

void heartbeat_init(void) {
  me = xbee_get_nw_address();
}

void heartbeat_step(void) {
  time_t now = clock_get_millis();
  
  // send a heartbeat
  if( now >= next_heartbeat ) {
    _beat();
    next_heartbeat += HEARTBEAT_INTERVAL;
  }

  now = clock_get_millis(); // refresh time, might bring next step closer ;-)
  
  // do background processing
  if( now >= next_processing ) {
    _process();
    next_processing += PROCESSING_INTERVAL;
  }
}

// processes payload received information
void heartbeat_receive(uint16_t source,
                       uint16_t from, uint16_t hop, uint16_t to,
                       uint8_t size, uint8_t *payload)
{
  // skip our own heartbeat messages
  if(from == me) { return; }

  // quick check if packet might be a heartbeat packet, based on size
  // it MUST be equal to PAYLOAD_SIZE
  if( size != PAYLOAD_SIZE ) { return; }

  heartbeat_node_t* node = _get_node(from);

  if( node == NULL ) { return; }    // out of storage :-(

  if( ! node->trust ) { return; }   // don't handle untrusted nodes

  // verify payload -> signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)payload, 5);
  
  if(sha1.result == shaSuccess &&
     memcmp(sha1.hash, &payload[5], SHA1HashSize) == 0)
  {
    node->seq  = payload[0];
    node->seen = clock_get_millis();
    _log_node("receive heartbeat", node);
  } else {
    node->incidents++;
    _log("FAILED sha1 check\n");
  }
}

// private helper functions

void _beat(void) {
  uint8_t payload[PAYLOAD_SIZE];

  // increase the heartbeat sequence number
  payload[0] = ++heartbeat;

  // get the time
  uint32_t millis = clock_get_millis();
  payload[1] = millis >> 24;
  payload[2] = millis >> 16;
  payload[3] = millis >>  8;
  payload[4] = millis;

  // create signature
  sha1_t sha1 = SHA1Compute((const uint8_t*)&(payload), 5);
  if(sha1.result == shaSuccess) {
    memcpy(&payload[5], &sha1.hash, SHA1HashSize);
  }

  _log("sending heartbeat : %02x %02x %02x %02x %02x\n",
       payload[0], payload[1], payload[2], payload[3], payload[4]);
  mesh_broadcast(me, PAYLOAD_SIZE, payload);
}

// background processing: check if anything went wrong and do this in two steps
// check if node's heartbeat is late.
// after 3 incidents, mark the node as untrustworthy
void _process(void) {
  time_t now = clock_get_millis();

  for(uint8_t i=0; i<node_count; i++) {
    if(nodes[i].trust) {
      if( now - nodes[i].seen > HEARTBEAT_INTERVAL) {
        nodes[i].incidents++;
        _log_node("late heartbeat", &nodes[i]);
      }

      if( nodes[i].incidents >= MAX_INCIDENTS ) {
        nodes[i].trust = FALSE;
        _log_node("trust lost", &nodes[i]);
      }
    }
  }
}

// NOTE this is a very basic implementation, we might need to implement it with
//      a tree or a hashtable based on the node's address
//      since the number of nodes is at most 3, this loop is as good ;-)
static heartbeat_node_t* _get_node(uint16_t address) {
  for(uint8_t i=0; i<MAX_NODES; i++) {
    if(nodes[i].address == address) { return &nodes[i]; }
  }
  // unknown node, create a new one and return that
  if(node_count >= MAX_NODES) {
    printf("FAIL: max nodes storage reached.\n");
    return NULL;
  }
  nodes[node_count].address   = address;
  nodes[node_count].seq       = 0;
  nodes[node_count].seen      = 0;
  nodes[node_count].incidents = 0;
  nodes[node_count].trust     = TRUE;
  _log_node("new node", &nodes[node_count]);
  
  nodes[node_count].address = address;
  return &nodes[node_count++];
}

static void _log_node(const char* msg, heartbeat_node_t* node) {
  _log("HB: %s : %02x %02x = seq: %d seen: %lu incidents: %d trust: %d\n",
       msg,
       (uint8_t)(node->address >> 8), (uint8_t)node->address,
       node->seq, node->seen, node->incidents, node->trust);
}
