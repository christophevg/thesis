// avr.h
// author: Christophe VG <contact@christophe.vg>

// two simple macros to turn on/off bits on a PORT/PIN
// taken from tutorial by Sparkfun
#define avr_set_bit(var, mask) ((var) |= (uint8_t)(1 << mask))
#define avr_clear_bit(var, mask) ((var) &= (uint8_t)~(1 << mask))

// elementary functions for handling the AVR/ATMEGA MCU
void     avr_init(void);
void     avr_adc_init(void);
uint16_t avr_adc_read(uint8_t ch);
uint16_t avr_get_vcc(void);

#if ! defined F_CPU     // should come in from the Makefile
#define F_CPU  18000000
#endif

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define MCU_NAME STR(MCU)

// mapping of functions -> actual ports
#if MCU == atmega1284p

// MISO
#define MISO_PORT DDRB
#define MISO_PIN PB6
// RX
#define RX_PORT DDRD
#define RX_PIN PD0

#else

// MISO
#define MISO_PORT DDRB
#define MISO_PIN PB4
// RX
#define RX_PORT DDRD
#define RX_PIN PD0

#endif
