// heartbeat.h
// author: Christophe VG

#ifndef __HEARTBEAT_H
#define __HEARTBEAT_H

// public interface
void heartbeat_init(void);
void heartbeat_step(void);
void heartbeat_receive(uint16_t from, uint16_t hop, uint16_t to,
                       uint8_t size, uint8_t *payload);

#endif
