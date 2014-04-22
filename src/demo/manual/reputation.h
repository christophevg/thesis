// reputation.h
// author: Christophe VG

#ifndef __REPUTATION_H
#define __REPUTATION_H

// public interface
void reputation_init(void);
void reputation_step(void);
void reputation_receive(uint16_t from, uint16_t hop, uint16_t to,
                       uint8_t size, uint8_t *payload);
void reputation_transmit(uint16_t from, uint16_t hop, uint16_t to,
                         uint8_t size, uint8_t *payload);

#endif
