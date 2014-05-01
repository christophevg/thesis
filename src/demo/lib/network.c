// network.c
// simple wrappers around frame-based xbee_send functionality
// author: Christophe VG

#include "network.h"

#include "moose/xbee.h"
#include "moose/random.h"

#include "../lib/log.h"

// XBee doesn't provide access to raw frame information for frames sent between
// other nodes (aka promiscuous mode). this module simulates this by sending
// packets to all nodes involved.

// the setup consists of three nodes:
// end-device - router - coordinator

// the end-device sends out information to the coordinator
//                broadcasts a heartbeat

// the router sends out information to the coordinator
//            broadcasts a heartbeat
//            forwards information from end-device to coordinator
//                     heartbeats  from end-device to coordinator

// the end-device knows about the router, because it's its parent
// the router doesn't know about the end-device ... until it receives a frame
// coming from it.

uint16_t me;
uint16_t parent;
bool     router;

volatile uint64_t other_address    = XB_COORDINATOR;
volatile uint16_t other_nw_address = XB_NW_ADDR_UNKNOWN;

// during intialisation, a broadcast packet is sent out to distribute our 
// address, we do this only from the end-device to initiate its JOINing
void mesh_init(void) {
  // cache own and parent's nw address
  me     = xbee_get_nw_address();
  parent = xbee_get_parent_address();
  router = parent == XB_NW_ADDR_UNKNOWN;
  
  if(router) { return; } // only end-nodes request to join

  // while we didn't receive any frames from the other side and could record
  // it's address, we keep sending out broadcasts
  while(other_address == XB_COORDINATOR) {
    // _log("join...\n");
    
    xbee_tx_t frame;
    frame.size        = 4;
    frame.id          = XB_TX_NO_RESPONSE;
    frame.address     = XB_BROADCAST;
    frame.nw_address  = XB_NW_BROADCAST;
    frame.radius      = 0x01;
    frame.options     = XB_OPT_NONE;
    frame.data        = (uint8_t*)"join";
    xbee_send(&frame);

    for(uint8_t l=0; l<10 && other_address == XB_COORDINATOR; l++) {
      _delay_ms(100L);
      xbee_receive();
    }
  }
}

bool mesh_child_connected(void) { return other_address != XB_COORDINATOR; }

static void _send(uint64_t address, uint16_t nw_address,
                  uint16_t from, uint16_t hop, uint16_t to,
                  uint8_t size, uint8_t* payload)
{
  uint8_t* bytes = malloc(3*sizeof(uint16_t)+size);
  // add broadcast hop and destination
  bytes[0] = (uint8_t)(from >> 8);  bytes[1] = (uint8_t)(from);
  bytes[2] = (uint8_t)(hop  >> 8);  bytes[3] = (uint8_t)(hop );
  bytes[4] = (uint8_t)(to   >> 8);  bytes[5] = (uint8_t)(to  );

  // add the actual payload
  memcpy(&(bytes[6]), payload, size);

  xbee_tx_t frame;
  frame.size        = size + 3*2;
  frame.id          = XB_TX_NO_RESPONSE;
  frame.address     = address;
  frame.nw_address  = nw_address;
  frame.radius      = 0x01;               // only go to parent/hop
  frame.options     = XB_OPT_NONE;
  frame.data        = bytes;
  xbee_send(&frame);  
}

mesh_tx_handler_t tx_handler = NULL;
void mesh_on_transmit(mesh_tx_handler_t handler) { tx_handler = handler; }

// sending message will require the message to be send to the destination (only
// the coordinator is a possible destination)
void mesh_send(uint16_t from, uint16_t to, uint8_t size, uint8_t* payload) {
  uint16_t hop16  = parent;
  uint64_t hop64;
  if(router) {                // we're the router, parent = coordinator
    hop16 = XB_COORDINATOR;
    hop64 = XB_COORDINATOR;
  } else {
    hop64 = other_address;    // other == router address, hop16 == parent
  }
  _send(hop64, hop16, from, hop16, to, size, payload);

  if(tx_handler != NULL) {
    tx_handler(from, hop16, to, size, payload);
  }

  // if we're a router, we need to send a copy to our child
  if(router && other_nw_address != XB_NW_ADDR_UNKNOWN) { 
    // _log("sending copy to child %02x %02x\n",
    //      (uint8_t)(other_nw_address >> 8), (uint8_t)other_nw_address);
    _send(other_address, other_nw_address, from, hop16, to, size, payload);
  }
}

void mesh_broadcast(uint16_t from, uint8_t size, uint8_t* payload) {
  mesh_send(from, XB_NW_BROADCAST, size, payload);
}

mesh_rx_handler_t rx_handler = NULL;
void mesh_on_receive(mesh_rx_handler_t handler) { rx_handler = handler; }

void mesh_receive(xbee_rx_t* frame) {
  // if this is the first message (== other node), cache its addresses
  if(other_nw_address == XB_NW_ADDR_UNKNOWN) {
    other_address    = frame->address;
    other_nw_address = frame->nw_address;
  }

  // don't further process broadcast from end-device = join
  if(frame->options == 0x42) { return; }

  uint16_t source = frame->nw_address;
  // parse additional routing information
  uint16_t from   = frame->data[1] | frame->data[0] << 8;
  uint16_t hop    = frame->data[3] | frame->data[2] << 8;
  uint16_t to     = frame->data[5] | frame->data[4] << 8;

  if(rx_handler != NULL) {
    rx_handler(source, from, hop, to, frame->size-6, &(frame->data[6]));
  }

  // if we're a router and not the final destination, pass it on (to our parent)
  // take into account a failure percentage
  if(to != me && router && rnd(100) > FORWARD_FAILURE_PCT) {
    // _log("forwarding message from %02x %02x to %02x %02x\n",
    //      (uint8_t)(from >> 8), (uint8_t)from, (uint8_t)(to >> 8), (uint8_t)to);
    mesh_send(from, to, frame->size-6, &(frame->data[6]));
  }
}
