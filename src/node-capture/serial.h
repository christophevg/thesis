// compute baud rate setting
#define BAUD   9600
#define MYUBRR (F_CPU/16/BAUD-1)

#include <stdio.h>
#include <avr/io.h>

void       serial_init(void);
static int serial_putchar(char c, FILE *stream);
uint8_t    serial_getchar(void);
