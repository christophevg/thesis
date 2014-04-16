// main.c
// author: Christophe VG

// main event loop for a wireless sensor node
// - ATMEGA1284p
// - RS-232 connection on USART1
// - XBEE on USART0 (XBee includes two LEDs indicating state and association)
// - light sensor
// - LEDs for power-on (red) / status (green)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "moose/avr.h"
#include "moose/bool.h"
#include "moose/serial.h"
#include "moose/sleep.h"
#include "moose/xbee.h"
#include "moose/clock.h"

#include "log.h"
#include "timing.h"
#include "network.h"
#include "heartbeat.h"

#include "hardware.h"

// forward declarations
void init(void);
void sleep(void);
void receive(xbee_rx_t*);

void application_step(void) {
  uint16_t reading;               // the 16-bit reading from the ADC
  uint8_t  values[2];             // the bytes containing the reading in bytes

  // read light sensor
  reading = avr_adc_read(LIGHT_SENSOR_PIN);
  values[0] = (reading >> 8 );
  values[1] = reading;

  // send it to the coordinator
  send_bytes(values, 2);
}

int main(void) {
  init();

  xbee_on_receive(receive);

  while(TRUE) {
    application_step();

    measure(
      heartbeat_step()
    );

    xbee_receive();
    sleep();
  }

  return(0);
}

void init(void) {
  // MCU
  avr_init();                     // initialise MCU
  avr_set_bit(STATUS_LED_PORT,    // turn on the red status led
              STATUS_LED_PIN);
  avr_adc_init();                 // initialize the ADC for normal readings

  sleep_init();                   // we're using power-down-style sleeping
  clock_init();                   // init/start the millis clock

  // SERIAL
  serial_init();                  // initialize use of serial port(s)
  log("serial initialized\n"); // announce boot process via serial

  // XBEE
  xbee_init();                    // initialize use of XBee module
  log("xbee initialized, waiting for association...\n");
  xbee_wait_for_association();    // wait until the network is available
  log("xbee associated\n");    // announce boot process via serial

  // FUNCTIONALITY
  avr_clear_bit(LIGHT_SENSOR_PORT,// make light sensor pin an input pin
                LIGHT_SENSOR_PIN);  
  avr_set_bit(VCC_SENSOR_PORT,    // provide power to sensor
              VCC_SENSOR_PIN);
}

void sleep(void) {
  avr_clear_bit(STATUS_LED_PORT,  // turn off the green status led
                STATUS_LED_PIN);
  _delay_ms(1000L);
  avr_set_bit(STATUS_LED_PORT,      // turn on the green status led
              STATUS_LED_PIN);
}

// frames that are received, are presented to all possible interested parties
// in this case: heartbeat
void receive(xbee_rx_t *frame) {
  // log("RX : from : %02x %02x | size : %i\n",
  //      (uint8_t)(frame->nw_address >> 8),
  //      (uint8_t)frame->nw_address,
  //      frame->size);
  // printf("-----------------------------");
  // for(uint8_t i=0; i<frame->size; i++) {
  //   if(i % 10 == 0) { printf("\n"); }
  //   printf("%02x ", frame->data[i]);
  // }
  // printf("\n-----------------------------\n");
  
  // present it to the ids hearbeat module
  measure(
    heartbeat_receive(frame->nw_address, frame->size, frame->data);
  );
}
