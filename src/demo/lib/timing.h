// timing.h
// support for timing using an output pin
// author: Christophe VG

#ifndef __TIMING_H
#define __TIMING_H

#define TIMING_PORT        PORTB      // PB1
#define TIMING_PIN         1

#define START avr_set_bit  (TIMING_PORT, TIMING_PIN)
#define STOP  avr_clear_bit(TIMING_PORT, TIMING_PIN)

#define measure(code) START; code; STOP;

#endif
