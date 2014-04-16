// network.h
// simple wrappers around frame-based xbee_send functionality
// author: Christophe VG

#ifndef __NETWORK_H
#define __NETWORK_H

#include <stdint.h>

// network config
#define DESTINATION XB_COORDINATOR

void send_bytes(uint8_t *bytes, uint8_t size);
void broadcast_bytes(uint8_t *bytes, uint8_t size);

#endif
