// nodes
// nodes related functions, specific for the moose platform
// demo-specific implementation of moose-specific parts
// -> using mesh_ functions to use virtual demo network
// author: Christophe VG

#include <string.h> // for memcpy: can be avoided

#include "moose/xbee.h"

#include "foo-lib/stdarg-collect.h"

#include "foo-lib/external.h"
#define NODES_T_H_name STR(NODES_T_H)
#ifdef NODES_T_H
#include STR(NODES_T_H)
#endif
#include "foo-lib/nodes.h"

#include "network.h"

/*
 * This implementation adds a bit of additional functionality to simulate
 * multi-hop messaging. Routing is made explicit by a statically configured
 * next hop. If a destination is NOT the next hop, the message is send to the
 * next hop. This next hop is also explicitly added as the first 64 bits of the
 * payload and the final destination is added as the second 64 bits of the
 * payload. All messaging is done through broadcasting, which allows us to
 * identify if a packet was meant for us, or if we overheart communication
 * between two other nodes. If a real broadcast is send, the final destination
 * and the hop are set to the default broadcast node.
 */

// forward declarations of private helper functions
void _broadcast(void);
void _send(void);

// our fixed next hop
node_t* next_hop;

// broadcasted messages are collected into a buffer  

// because we add 3*2 bytes for routing information, the maximum amount of bytes
// that can be send/collected is 104-6 = 98

#define MAX_SIZE 98

uint8_t broadcast_buffer[MAX_SIZE];
uint8_t broadcast_size = 0;

// we keep a buffer for each possible node. this implementation should be more
// dynamic, to accomodate for large numbers of nodes. here we can simply
uint8_t send_buffer[MAX_NODES][MAX_SIZE];
uint8_t send_size[MAX_NODES];

void nodes_broadcast(uint16_t size, ...) {
  if(size > MAX_SIZE-broadcast_size) { return; } // TODO: dont' drop silently
  uint8_t* buffer = &(broadcast_buffer[broadcast_size]);
  va_collect(buffer, size, uint8_t);
  broadcast_size += size;
}

void nodes_send(node_t* node, uint16_t size, ...) {
  if(size > MAX_SIZE-send_size[node->id]) { return; }//TODO: dont' drop silently
  uint8_t* buffer = &(send_buffer[node->id][broadcast_size]);
  va_collect(buffer, size, uint8_t);
  send_size[node->id] += size;
}

void nodes_io_init(void) {}

void nodes_io_process(void) {
  _broadcast();
  _send();
  xbee_receive();
}

uint16_t nodes_get_nw_address(void) {
  return xbee_get_nw_address();
}

// private helper functions

void _broadcast(void) {
  if(broadcast_size > 0) {
    mesh_broadcast(xbee_get_nw_address(), broadcast_size, broadcast_buffer);
    broadcast_size = 0;
  }
}

void _send(void) {
  for(uint8_t i=0; i<nodes_count(); i++) {
    if(send_size[i] > 0) {
      mesh_send(xbee_get_nw_address(), nodes_get(i)->address, send_size[i], send_buffer[i]);
      send_size[i] = 0;
    }
  }
}
