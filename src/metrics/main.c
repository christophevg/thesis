// main.c
// author: Christophe VG

// experiment to construct metering infrastructure

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "moose/avr.h"
#include "moose/bool.h"
#include "moose/serial.h"
#include "moose/sleep.h"
#include "moose/xbee.h"
#include "moose/clock.h"

// configuration of external components
#define STATUS_LED_PORT    PORTB  // PB0
#define STATUS_LED_PIN     0

#define TIMING_PORT    PORTB      // PB1
#define TIMING_PIN     1

// network config
#define DESTINATION XB_COORDINATOR

// forward declarations
void init(void);
void send_bytes(uint8_t *bytes, uint8_t size);
void broadcast_bytes(uint8_t *bytes, uint8_t size);
void send_str(const char *string);
void sleep(void);
void receive(xbee_rx_t*);

// little hack to have a timed log function without the function ;-)
#define log printf("[%06lu] ", clock_get_millis()); printf

int main(void) {
  init();

  xbee_on_receive(receive);

  while(TRUE) {
    
    // start timing
    avr_set_bit(TIMING_PORT, TIMING_PIN);

    // process receiving
    xbee_receive();
    
    // stop timing
    avr_clear_bit(TIMING_PORT, TIMING_PIN);

    // and go to sleep for a while...
    _delay_ms(2000L);
  }

  return(0);
}

void init(void) {
  // MCU
  avr_init();                     // initialise MCU
  avr_set_bit(STATUS_LED_PORT,    // turn on the green status led
              STATUS_LED_PIN);
  avr_adc_init();                 // initialize the ADC for normal readings

  sleep_init();                   // we're using power-down-style sleeping
  clock_init();                   // init/start the millis clock

  // SERIAL
  serial_init();                  // initialize use of serial port(s)
  printf("-----------------------------------------------------------------\n");
  printf("serial initialized\n"); // announce boot process via serial

  // XBEE
  xbee_init();                    // initialize use of XBee module
  log("xbee initialized, waiting for association...\n");
  xbee_wait_for_association();    // wait until the network is available
  log("xbee associated\n");    // announce boot process via serial
  send_str("xbee associated\n");  // announce boot process via xbee
}

void send_str(const char *string) {
  send_bytes((uint8_t*)string, strlen(string));
}

void send_bytes(uint8_t *bytes, uint8_t size) {
  xbee_tx_t frame;

  frame.size        = size;
  frame.id          = XB_TX_NO_RESPONSE;
  frame.address     = DESTINATION;
  frame.nw_address  = XB_NW_ADDR_UNKNOWN;
  frame.radius      = XB_MAX_RADIUS;
  frame.options     = XB_OPT_NONE;
  frame.data        = bytes;

  xbee_send(&frame);
}

void broadcast_bytes(uint8_t *bytes, uint8_t size) {
  xbee_tx_t frame;

  frame.size        = size;
  frame.id          = XB_TX_NO_RESPONSE;
  frame.address     = XB_BROADCAST;
  frame.nw_address  = XB_NW_BROADCAST;
  frame.radius      = XB_MAX_RADIUS;
  frame.options     = XB_OPT_NONE;
  frame.data        = bytes;

  xbee_send(&frame);
}

// frames that are received, are presented to all possible interested parties
// in this case: ids ;-)
void receive(xbee_rx_t *frame) {
  log("RX : from : %02x %02x | size : %i\n",
       (uint8_t)(frame->nw_address >> 8),
       (uint8_t)frame->nw_address,
       frame->size);
  printf("-----------------------------");
  for(uint8_t i=0; i<frame->size; i++) {
    if(i % 10 == 0) { printf("\n"); }
    printf("%02x ", frame->data[i]);
  }
  printf("\n-----------------------------\n");
}
