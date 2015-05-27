// reputation.h
// author: Christophe VG

#ifndef __REPUTATION_H
#define __REPUTATION_H

#define VALIDATION_INTERVAL 5000    // interval to check trust of nodes
#define SHARING_INTERVAL    7500    // interval to broadcast reputation info

// public interface
void reputation_init(void);
void reputation_share(void);
void reputation_validate(void);
void reputation_receive(uint16_t source,
                        uint16_t from, uint16_t hop, uint16_t to,
                        uint8_t size, uint8_t *payload);
void reputation_transmit(uint16_t from, uint16_t hop, uint16_t to,
                         uint8_t size, uint8_t *payload);

#endif
