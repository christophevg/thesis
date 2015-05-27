// heartbeat.h
// author: Christophe VG

#ifndef __HEARTBEAT_H
#define __HEARTBEAT_H

#define HEARTBEAT_INTERVAL  3000  // send out a heartbeat every 3s
#define PROCESSING_INTERVAL 5000  // process every 1s

// public interface
void heartbeat_init(void);
void heartbeat_send(void);
void heartbeat_process(void);
void heartbeat_receive(uint16_t source,
                       uint16_t from, uint16_t hop, uint16_t to,
                       uint8_t size, uint8_t *payload);

#endif
