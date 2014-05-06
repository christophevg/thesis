#include "../lib/hardware.h"
#include "../lib/log.h"
#include "../lib/network.h"
#include "../lib/timing.h"

// our own address and that of our parent node
uint16_t address;
node_t*  me;        // global reference to "me"
uint16_t parent;

#define METRICS_REPORT_INTERVAL 15000

void report_metrics(void) {
         time_t   now         = clock_get_millis();
  static time_t   next_report = 0;

  // could event-loop cycles
  static unsigned long cycles      = 0;
  cycles++;

  if(next_report == 0) {
    next_report = now + METRICS_REPORT_INTERVAL;
  }
  static unsigned long total_frames  = 0,
                       total_bytes   = 0,
                       samples       = 0;
  if(next_report < now) {

    xbee_metrics_t metrics = xbee_reset_counters();
    total_frames += metrics.frames;
    total_bytes  += metrics.bytes;
    samples++;

    _log("metrics: cycles: %lu (ev:%u us) | xbee: %d frames (avg:%u/tot:%lu) / %i bytes (avg:%u/tot:%lu)\n",
         cycles, (unsigned int)((now * 1000.0) / cycles),
         metrics.frames, (unsigned int)(total_frames / samples), total_frames,
         metrics.bytes,  (unsigned int)(total_bytes  / samples), total_bytes);

    next_report += METRICS_REPORT_INTERVAL;
  }
}
