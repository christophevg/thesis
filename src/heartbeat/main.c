// main.c
// author: Christophe VG

// main event loop for a wireless sensor node
// - ATMEGA1284p
// - RS-232 connection on USART1
// - XBEE on USART0 (XBee includes two LEDs indicating state and association)
// - light sensor
// - LEDs for power-on (red) / status (green)  / alert (orange)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "../an-avr-lib/avr.h"
#include "../an-avr-lib/bool.h"
#include "../an-avr-lib/serial.h"
#include "../an-avr-lib/sleep.h"
#include "../an-avr-lib/xbee.h"
#include "../an-avr-lib/clock.h"

// configuration of external components

#define STATUS_LED_PORT    PORTB  // PB0
#define STATUS_LED_PIN     0

#define ALERT_LED_PORT     PORTB  // PB1
#define ALERT_LED_PIN      1

#define LIGHT_SENSOR_PORT  DDRA   // PA0
#define LIGHT_SENSOR_PIN   0

#define VCC_SENSOR_PORT    PORTA  // PB7
#define VCC_SENSOR_PIN     7

// network config
#define DESTINATION XBEE_COORDINATOR

// forward declarations
void init(void);
void send_bytes(uint8_t *bytes, uint8_t size);
void send_str(const char *string);
void sleep(void);
void receive(xbee_rx_t*);

int main(void) {
  uint16_t reading;               // the 16-bit reading from the ADC
  uint8_t  values[2];             // the bytes containing the reading in bytes

  init();
  xbee_on_receive(receive);

  while(TRUE) {
    // light
    reading = avr_adc_read(LIGHT_SENSOR_PIN);
    values[0] = (reading >> 8 ) & 0x00FF;
    values[1] = reading & 0x00FF;

    // send it to the coordinator
    send_bytes(values, 2);
    // print it to the serial
    printf("[%06lu] light = %3i (0x%04X)\n", clock_get_millis(),
           reading, reading);

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
  printf("-----------------------------------------------------------------\n");
  printf("serial initialized\n"); // announce boot process via serial

  // XBEE
  xbee_init();                    // initialize use of XBee module
  printf("xbee initialized, waiting for association...\n");
  xbee_wait_for_association();    // wait until the network is available
  printf("xbee associated\n");    // announce boot process via serial
  send_str("xbee associated\n");  // announce boot process via xbee

  // FUNCTIONALITY
  avr_clear_bit(LIGHT_SENSOR_PORT,// make light sensor pin an input pin
                LIGHT_SENSOR_PIN);  
  avr_set_bit(VCC_SENSOR_PORT,    // provide power to sensor
              VCC_SENSOR_PIN);
}

void send_str(const char *string) {
  send_bytes((uint8_t*)string, strlen(string));
}

void send_bytes(uint8_t *bytes, uint8_t size) {
  xbee_tx_t frame;

  frame.size        = size;
  frame.id          = XB_TX_NO_RESPONSE;
  frame.address     = XB_COORDINATOR;
  frame.nw_address  = XB_NW_ADDR_UNKNOWN;
  frame.radius      = XB_MAX_RADIUS;
  frame.options     = XB_OPT_NONE;
  frame.data        = bytes;

  xbee_send(&frame);
}

void sleep(void) {
  avr_clear_bit(STATUS_LED_PORT,  // turn off the green status led
                STATUS_LED_PIN);
  _delay_ms(1000L);
  avr_set_bit(STATUS_LED_PORT,      // turn on the green status led
              STATUS_LED_PIN);
}

void receive(xbee_rx_t *frame) {
  printf("[%lu] RX : %i / %i\n", clock_get_millis(), frame->nw_address, frame->size);
  printf("      ");
  for(int i=0; i<frame->size; i++) {
    printf("%i ", frame->data[i]);
  }
  printf("\n");
}
