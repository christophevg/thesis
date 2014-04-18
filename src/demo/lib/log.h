// log.h
// little hack to have a timed log function without the function ;-)
// author: Christophe VG

#ifndef log
#include <stdio.h>
#include "moose/clock.h"
#define log printf("[%06lu] ", clock_get_millis()); printf
#endif
