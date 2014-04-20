// log.h
// little hack to have a timed log function without the function ;-)
// author: Christophe VG

#ifndef __LOG_H
#define __LOG_H
#include <stdio.h>
#include "moose/clock.h"
#define _log printf("[%06lu] ", clock_get_millis()); printf
#endif
