#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "LPC11xx.h"


// SYSAHBCLKCTRL bits
#define AHBCLKCTRL_I2C (1<<5)
#define AHBCLKCTRL_IOCON (1<<16)

// Pin functions
#define PIN_PIO_FUNC 0x0
#define PIN_I2C_FUNC 0x1

// Pin I2C modes
#define I2CMODE_STANDARD_FAST (0x3 << 8)
#define I2CMODE_STANDARD_IO		(0x1 << 8)
#define I2CMODE_FAST_PLUS (0x2 << 8)

// Reset control I2C bit
#define I2C_RST_N (1<<1)

//I2C CON bits
#define I2EN (1<<6)
#define STA (1<<5)
#define STO (1<<4)
#define SI (1<<3)
#define AA (1<<2)



// Stuff from library
#define I2C_READ    	1
#define I2C_WRITE   	0

// I2C Task Synchronization Struct
typedef struct I2CTask {
	uint32_t address;					// I2C slave address - only 32 bits because C is dumb sometimes
	uint8_t action;						// I2C_READ, I2C_WRITE
	volatile uint8_t *buffer; // buffer of bytes to be transmitted over I2C
	uint32_t byte_count;			// length of buffer
	uint32_t current_byte;		// how many bytes transmitted so far
	uint8_t is_busy;					// Used in order to make I2C commands blocking - easy synchronization

} I2CTask_t;


void I2C_init(void);
void I2C_write(uint32_t address, uint8_t *buffer, uint32_t count);
void I2C_read(uint32_t address, uint8_t *buffer, uint32_t count);

#endif /* I2C_H */
