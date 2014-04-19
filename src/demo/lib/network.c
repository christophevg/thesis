// network.c
// simple wrappers around frame-based xbee_send functionality
// author: Christophe VG

#include "network.h"

#include "moose/xbee.h"

#include "../lib/log.h"

void mesh_send(uint16_t from, uint16_t to, uint8_t size, uint8_t* payload) {
  uint16_t hop = xbee_get_parent_address();
  if(hop == XB_NW_ADDR_UNKNOWN) {   // we're the router, parent = coordinator
    hop = XB_COORDINATOR;
  }

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
  frame.address     = XB_BROADCAST;
  frame.nw_address  = XB_NW_BROADCAST;
  frame.radius      = 0x01;               // only go to parent/hop
  frame.options     = XB_OPT_NONE;
  frame.data        = bytes;
  xbee_send(&frame);
}

void mesh_broadcast(uint16_t from, uint8_t size, uint8_t* payload) {
  mesh_send(from, XB_NW_BROADCAST, size, payload);
}

mesh_rx_handler_t rx_handler;

void mesh_on_receive(mesh_rx_handler_t handler) {
  rx_handler = handler;
}

void mesh_receive(xbee_rx_t* frame) {
  uint16_t source = frame->nw_address;
  uint16_t from   = frame->data[1] | frame->data[0] << 8;
  uint16_t hop    = frame->data[3] | frame->data[2] << 8;
  uint16_t to     = frame->data[5] | frame->data[4] << 8;

  rx_handler(source, from, hop, to, frame->size-6, &(frame->data[6]));

  // if we're not the addressee, pass it on to our parent (if we're a router)
  if(to != xbee_get_nw_address() && xbee_get_parent_address() == 0xfffe) {
    log("forwarding message from %02x %02x to %02x %02x\n",
        (uint8_t)(from >> 8), (uint8_t)from, (uint8_t)(to >> 8), (uint8_t)to);
    mesh_send(from, to, frame->size-6, &(frame->data[6]));
  }
}
