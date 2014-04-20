// network.c
// simple wrappers around frame-based xbee_send functionality
// author: Christophe VG

#include "network.h"

#include "moose/xbee.h"

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

volatile uint64_t other_address    = XB_COORDINATOR;
volatile uint16_t other_nw_address = XB_NW_ADDR_UNKNOWN;

// during intialisation, a broadcast packet is sent out to distribute our 
// address, we do this only from the end-device to initiate its JOINing
void mesh_init(void) {
  if(xbee_get_parent_address() == XB_NW_ADDR_UNKNOWN) { return; } // router

  // while we didn't receive any frames from the other side and could record
  // it's address, we keep sending out broadcasts
  while(other_address == XB_COORDINATOR) {
    _log("join...\n");
    uint8_t bytes[] = { 'J', 'O', 'I', 'N' };
    
    xbee_tx_t frame;
    frame.size        = 4;
    frame.id          = XB_TX_NO_RESPONSE;
    frame.address     = XB_BROADCAST;
    frame.nw_address  = XB_NW_BROADCAST;
    frame.radius      = 0x01;
    frame.options     = XB_OPT_NONE;
    frame.data        = bytes;
    xbee_send(&frame);

    for(uint8_t l=0; l<10 && other_address == XB_COORDINATOR; l++) {
      _delay_ms(100L);
      xbee_receive();
    }
  }
}

static void _send(uint64_t address, uint16_t nw_address,
                  uint16_t from, uint16_t hop, uint16_t to,
                  uint8_t size, uint8_t* payload)
{
  uint8_t* bytes = malloc(3*sizeof(uint16_t)+size);
  // add broadcast hop and destination
  bytes[0] = (uint8_t)(from >> 8);  bytes[1] = (uint8_t)(from);
  bytes[2] = (uint8_t)(hop >> 8);   bytes[3] = (uint8_t)(hop);
  bytes[4] = (uint8_t)(to >> 8);    bytes[5] = (uint8_t)(to);

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

// sending message will require the message to be send to the destination (only
// the coordinator is a possible destination)
void mesh_send(uint16_t from, uint16_t to, uint8_t size, uint8_t* payload) {
  uint16_t parent = xbee_get_parent_address(),
           hop    = parent;
  uint64_t via;
  if(hop == XB_NW_ADDR_UNKNOWN) {   // we're the router, parent = coordinator
    hop = XB_COORDINATOR;
    via = XB_COORDINATOR;
  } else {
    via = other_address;            // other == router address
  }
  // send it
  _send(via, XB_NW_ADDR_UNKNOWN,
        from, hop, to,
        size, payload);

  // if we're a router, we need to send a copy to our child
  if(parent == XB_NW_ADDR_UNKNOWN && other_nw_address != XB_NW_ADDR_UNKNOWN) { 
    _log("dup to child\n");
    _send(other_address, other_nw_address,
          from, hop, to,
          size, payload);
  }
}

void mesh_broadcast(uint16_t from, uint8_t size, uint8_t* payload) {
  mesh_send(from, XB_NW_BROADCAST, size, payload);
}

mesh_rx_handler_t rx_handler;

void mesh_on_receive(mesh_rx_handler_t handler) {
  rx_handler = handler;
}

void mesh_receive(xbee_rx_t* frame) {
  if(frame->options == 0x42) {  // broadcast | end-device
    // end-node trying to join, record it's information
    other_address    = frame->address;
    other_nw_address = frame->nw_address;
    return;
  }

  if(frame->size < 7) {
    _log("FRAME TOO SMALL: %i\n", frame->size);
    return;
  }
  
  uint16_t source = frame->nw_address;

  uint16_t from   = frame->data[1] | frame->data[0] << 8;
  uint16_t hop    = frame->data[3] | frame->data[2] << 8;
  uint16_t to     = frame->data[5] | frame->data[4] << 8;

  // cache other side's address (end-node <-> router)
  if(other_nw_address == XB_NW_ADDR_UNKNOWN) {
    other_address    = frame->address;
    other_nw_address = from;
  }

  rx_handler(source, from, hop, to, frame->size-6, &(frame->data[6]));

  // if we're not the addressee, pass it on to our parent (if we're a router)
  if(to != xbee_get_nw_address() && xbee_get_parent_address() == 0xfffe) {
    _log("forwarding message from %02x %02x to %02x %02x\n",
         (uint8_t)(from >> 8), (uint8_t)from, (uint8_t)(to >> 8), (uint8_t)to);
    mesh_send(from, to, frame->size-6, &(frame->data[6]));
  }
}
