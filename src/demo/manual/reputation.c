// reputation.c
// author: Christophe VG

// a node that sends out a message via a node that's not the final destination
// of the message, will check if the next hop actually forwards its message.

#include <string.h>

#include "moose/bool.h"
#include "moose/clock.h"
#include "moose/xbee.h"

#include "../lib/log.h"
#include "../lib/network.h"

#include "reputation.h"

// configuration

#define FORWARD_INTERVAL    1000    // forwards must be completed within 1s
#define VALIDATION_INTERVAL 5000    // interval to check trust of nodes
#define SHARING_INTERVAL    7500    // interval to broadcast reputation info
#define INDIRECT_THRESHOLD     0.9  // lower limit for including indirect info
#define AGING_WEIGHT           0.98 // factor to age reputation

// payload consists of 2 bytes for the network address of the node
//                 and 4 bytes for the float typed alpha param
//                 and 4 bytes for the float typed alpha param
//                  = 10 bytes payload
#define PAYLOAD_SIZE          10
#define MAX_NODES              5  // maximum number of tracked neighbours

// private type

typedef struct {
  uint8_t  size;
  uint8_t* data;
} payload_t;

typedef struct tracked {
  time_t          timeout;
  payload_t       payload;
  struct tracked* next;
} tracked_t;

typedef struct {
  uint16_t   address;    // the network address of the node
  tracked_t* queue;      // queue of tracked
  uint8_t    msg_count;  // number of messages expected to be forwarded
  uint8_t    incidents;  // counter for incidents
  float      alpha;      // params to determine reputation
  float      beta;
  float      trust;      // to trust or not to trust, that is the question
} reputation_node_t;


// internal data

static reputation_node_t nodes[MAX_NODES];
static uint8_t           node_count = 0;

// intervals markers
static time_t next_validation = 0;
static time_t next_sharing    = 0;

// our own cached address
uint16_t me;

// forward declarations of private helper functions
static void _validate(void);
static void _share(void);
static void _track(reputation_node_t* node, uint8_t size, uint8_t* payload);
static void _untrack(reputation_node_t* node, uint8_t size, uint8_t* payload);
static uint8_t _remove_late(reputation_node_t* node);
static reputation_node_t* _get_node(uint16_t address);
static void _log_node(const char* msg, reputation_node_t* node);

// public interface

void reputation_init(void) {
  me = xbee_get_nw_address();  
}

void reputation_step(void) {
  time_t now = clock_get_millis();
  
  // validate known nodes
  if( now >= next_validation ) {
    _validate();
    next_validation = now + VALIDATION_INTERVAL;
  }
  
  // share reputation information
  if( now >= next_sharing ) {
    _share();
    next_sharing = now + SHARING_INTERVAL;
  }
}

// processing all incoming messages.
// - reputation sharing info
// - other messages, forwarded messages to validate forwarding
//   forwarded means: from == me, 
void reputation_receive(uint16_t source, 
                        uint16_t from, uint16_t hop, uint16_t to,
                        uint8_t size, uint8_t *payload)
{
  reputation_node_t* sending_node = _get_node(source);
  if( sending_node == NULL ) { return; }  // out of storage

  // tracking of MY messages
  if( from == me ) {
    _untrack(sending_node, size, payload);
  } else if(size == PAYLOAD_SIZE) { // other options
    // a reputation message: parse node address, alpha and beta values.
    uint16_t address = ((uint16_t)(payload[0]) << 8) | payload[1];
    if(address == me) { return; } // we're not interested in other's opinion ;-)
    reputation_node_t* of = _get_node(address);
    union {
      float value;
      uint8_t b[4];
    } alpha, beta;
    alpha.b[0] = payload[2];
    alpha.b[1] = payload[3];
    alpha.b[2] = payload[4];
    alpha.b[3] = payload[5];
    beta.b[0]  = payload[6];
    beta.b[1]  = payload[7];
    beta.b[2]  = payload[8];
    beta.b[3]  = payload[9];

    if( sending_node->trust > INDIRECT_THRESHOLD ) {
      // taking into account of indirect reputation information
      float weight = (2 * sending_node->alpha) /
                     ( (sending_node->beta+2) * (alpha.value + beta.value + 2) 
                       * 2 * sending_node->alpha );
      of->alpha += weight * alpha.value;
      of->beta  += weight * beta.value;
    }
  }
}

void reputation_transmit(uint16_t from, uint16_t hop, uint16_t to,
                         uint8_t size, uint8_t *payload)
{
  if( hop == to ) { return; } // final destination, no forward expected
  if( hop == XB_COORDINATOR ) { return; } // the coordinator doesn't forward
  
  // we expect to see this same payload again within the forward interval
  _log("RP: tracking payload from %02x %02x to %02x %02x : size=%d\n",
       (uint8_t)(hop >> 8), (uint8_t)hop, 
       (uint8_t)(to  >> 8), (uint8_t)to, size);
  reputation_node_t* node = _get_node(hop);
  if( node == NULL ) { return; }  // out of storage
  _track(node, size, payload);
}

// private helper functions

static void _validate(void) {
  _log("RP: validating %d nodes\n", node_count);
  
  for(uint8_t n=0; n<node_count; n++) {
    
    uint8_t failures = _remove_late(&nodes[n]);

    // update the reputation parameters
    nodes[n].alpha = (AGING_WEIGHT * nodes[n].alpha)
                   + nodes[n].msg_count - failures;
    nodes[n].beta  = (AGING_WEIGHT * nodes[n].beta )
                   + failures;

    // and compute trust
    nodes[n].trust = (nodes[n].alpha + 1)
                   / (nodes[n].alpha + nodes[n].beta + 2);

    _log("RP: validating node %d (%02x %02x) : failures=%d = alpha=%f beta=%f trust=%f\n",
         n, (uint8_t)(nodes[n].address >> 8), (uint8_t)nodes[n].address,
         failures, nodes[n].alpha, nodes[n].beta, nodes[n].trust);

    // notify bad node
    if(nodes[n].trust < 0.25) {
      _log("RP: trust lost\n");
      mesh_send(me, nodes[n].address, 8, (uint8_t*)"excluded");
    }

    // reset message counter
    nodes[n].msg_count = 0;
  }
}

static void _share(void) {
  _log("RP: sharing info on %d nodes\n", node_count);
  
  for(uint8_t n=0; n<node_count; n++) {
    union {
      struct {
        uint8_t node_msb;   // 1
        uint8_t node_lsb;   // 1
        float   alpha;      // 4
        float   beta;       // 4
      } values;
      uint8_t bytes[PAYLOAD_SIZE];
    } payload;
    payload.values.node_msb  = (uint8_t)nodes[n].address >> 8;
    payload.values.node_lsb  = (uint8_t)nodes[n].address;
    payload.values.alpha = nodes[n].alpha;
    payload.values.beta  = nodes[n].beta;
    mesh_broadcast(me, PAYLOAD_SIZE, payload.bytes);
  }
}

static void _track(reputation_node_t* node, uint8_t size, uint8_t* payload) {
  // create tracked payload structure
  tracked_t* tracked = malloc(sizeof(tracked_t));
  tracked->timeout = clock_get_millis() + FORWARD_INTERVAL;
  (tracked->payload).size = size;
  (tracked->payload).data = malloc(size*sizeof(uint8_t));
  memcpy((tracked->payload).data, payload, size);

  // add the tracked payload to the queue - works even with empty list ;-)
  tracked->next = node->queue;
  node->queue   = tracked;
}

static void _untrack(reputation_node_t* node, uint8_t size, uint8_t* payload) {
  tracked_t* tracked = node->queue;
  tracked_t* parent  = NULL;
  
  while(tracked != NULL) {
    if( (tracked->payload).size == size && 
        memcmp((tracked->payload).data, payload, size) == 0 )
    {
      // found payload, remove it
      
      if(parent == NULL) {
        node->queue = tracked->next;
      } else {
        parent->next = tracked->next;
      }
      _log("RP: cleared payload from %02x %02x : size=%d\n",
           (uint8_t)(node->address >> 8), (uint8_t)node->address, size);
      return;
    }
    parent  = tracked;
    tracked = tracked->next;
  }
  _log("RP: unexpected payload from %02x %02x\n",
       (uint8_t)(node->address >> 8), (uint8_t)node->address);
}

static uint8_t _remove_late(reputation_node_t* node) {
  tracked_t* tracked = node->queue;
  tracked_t* parent  = NULL;
  uint8_t    lates   = 0;

  while(tracked != NULL) {
    if( tracked->timeout < clock_get_millis() ) {
      _log("RP: late: %02x %02x : size=%d\n",
           (uint8_t)(node->address >> 8), (uint8_t)node->address,
           (tracked->payload).size);
      // this one is late
      lates++;
      if(parent == NULL) {
        node->queue = tracked->next;
      } else {
        parent->next = tracked->next;
      }
    }
    parent  = tracked;
    tracked = tracked->next;
  }
  return lates;
}

// NOTE this is a very basic implementation, we might need to implement it with
//      a tree or a hashtable based on the node's address
//      since the number of nodes is at most 3, this loop is as good ;-)
static reputation_node_t* _get_node(uint16_t address) {
  for(uint8_t i=0; i<node_count; i++) {
    if(nodes[i].address == address) { return &nodes[i]; }
  }
  // unknown node, create a new one and return that
  if(node_count >= MAX_NODES) {
    printf("FAIL: max nodes storage reached.\n");
    return NULL;
  }
  nodes[node_count].address   = address;
  nodes[node_count].queue     = NULL;
  nodes[node_count].msg_count = 0;
  nodes[node_count].incidents = 0;
  nodes[node_count].trust     = 0;
  _log_node("new node", &nodes[node_count]);
    
  return &nodes[node_count++];
}

static void _log_node(const char* msg, reputation_node_t* node) {
  _log("RP: %s : %02x %02x = msg_count: %d incidents: %d trust: %f\n",
       msg,
       (uint8_t)(node->address >> 8), (uint8_t)node->address,
       node->msg_count, node->incidents, (double)node->trust);
}
