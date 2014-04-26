// config.h
// author: Christophe VG

// compilation-time configuration of manual implementation

#define WITH_HEARTBEAT_name = STR(WITH_HEARTBEAT)
#define WITH_REPUTATION_name = STR(WITH_REPUTATION)

#ifdef WITH_HEARTBEAT
#pragma message "BUILDING WITH HEARTBEAT SUPPORT"
#endif

#ifdef WITH_REPUTATION
#pragma message "BUILDING WITH REPUTATION SUPPORT"
#endif
