// generated
typedef struct node_t {
  // domain properties
  uint16_t address;
  // extended properties for heartbeat
  uint8_t sequence;
  time_t last_seen;
  uint8_t fail_count;
  bool sane;
  // extended properties for reputation
  tuple_0_t* queue;
  uint8_t msg_count;
  float alpha;
  float beta;
  float trust;
} node_t;

// 2 + 1 + 4 + 1 + 1 + 2 + 1 + 4 + 4 + 4 = 25

// manual

typedef struct {
  uint16_t address;    // the network address of the node
  uint8_t  seq;        // last sequence id seen
  time_t   seen;       // the time when we saw the node (our time)
  uint8_t  incidents;  // counter for incidents
  bool     trust;      // to trust or not to trust, that is the question
} heartbeat_node_t;

// 2 + 1 + 4 + 1 + 1 = 9

typedef struct {
  uint16_t   address;    // the network address of the node
  tracked_t* queue;      // queue of tracked
  uint8_t    msg_count;  // number of messages expected to be forwarded
  uint8_t    incidents;  // counter for incidents
  float      alpha;      // params to determine reputation
  float      beta;
  float      trust;      // to trust or not to trust, that is the question
} reputation_node_t;

// 2 + 2 + 1 + 1 + 4 + 4 + 4 = 18

// total manual = 27
// generated    = 25
// gain         =  2 bytes = 7.4%


// with 64bits address
// manual     = 39
// generated  = 31
// gain       =  8 bytes = 20.5%