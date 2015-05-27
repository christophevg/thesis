#include "../lib/hardware.h"
#include "../lib/log.h"
#include "../lib/network.h"
#include "../lib/timing.h"

// our own address and that of our parent node
uint16_t address;
uint16_t parent;

#define METRICS_REPORT_INTERVAL 15000
#define RECEIVE_INTERVAL          100

// count event-loop cycles
unsigned long cycles = 0;

void report_metrics(void) {
  static unsigned long total_frames  = 0,
                       total_bytes   = 0,
                       samples       = 0;
  xbee_metrics_t metrics = xbee_reset_counters();
  total_frames += metrics.frames;
  total_bytes  += metrics.bytes;
  samples++;

  _log("metrics: cycles: %lu (ev:%u us) | xbee: %d frames (avg:%u/tot:%lu) / %i bytes (avg:%u/tot:%lu)\n",
       cycles, (unsigned int)((clock_get_millis() * 1000.0) / cycles),
       metrics.frames, (unsigned int)(total_frames / samples), total_frames,
       metrics.bytes,  (unsigned int)(total_bytes  / samples), total_bytes);
}
