// heartbeat.h
// author: Christophe VG

#ifndef __HEARTBEAT_H
#define __HEARTBEAT_H

// configuration
#define HEARTBEAT_MAX_NODES      5  // maximum number of tracked neighbours
#define HEARTBEAT_PAYLOAD_SIZE  25  // allows for constant length strings

// constant length payload byte-array
// consists of  1 byte for the heartbeat sequence
//         and  4 bytes for the node's time in millis
//         and 20 bytes for the signature = SHA1 hash over sequence + millis
//           = 25 bytes payload
typedef struct {
  uint8_t data[HEARTBEAT_PAYLOAD_SIZE];
} heartbeat_payload_t;

// struct to keep track of nodes' last seen time and status
typedef struct {
  uint16_t address;    // the network address of the node
  uint8_t  seq;        // last sequence id seen
  time_t   seen;       // the time when we saw the node (our time)
  time_t   time;       // the time the node announced   (their time)
  long     diff;       // the average difference between our and their time
  uint8_t  flags;      // bitfield, see flags below
} heartbeat_node_t;

// flags
#define HEARTBEAT_NODE_OK          0x00
#define HEARTBEAT_NODE_UNKNOWN     0x80
#define HEARTBEAT_NODE_SUSPICIOUS  0x40
#define HEARTBEAT_NODE_INVALID_SIG 0x20
#define HEARTBEAT_NODE_SEQ_SKIP    0x10

// interface
void heartbeat_receive(uint16_t source, uint8_t size, uint8_t *payload);
heartbeat_payload_t heartbeat_create_payload(void);
void heartbeat_process(void);

#endif
