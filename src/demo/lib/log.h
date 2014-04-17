// log.h
// little hack to have a timed log function without the function ;-)
// author: Christophe VG

// relies on clock_get_millis from moose/clock

#ifndef log
#define log printf("[%06lu] ", clock_get_millis()); printf
#endif
