#include <Arduino.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "i2c.h"

#define I2C_START     0x08
#define I2C_START_RPT 0x10
#define I2C_SLA_W_ACK 0x18
#define I2C_SLA_R_ACK 0x40
#define I2C_DATA_ACK  0x28

uint8_t i2cStart()
{
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))) ;
	return (TWSR & 0xF8);
}

void i2c_end()
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	while ((TWCR & (1<<TWSTO))) ;
}

bool i2c_write(uint8_t data)
{
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))) ;
  uint8_t ret = TWSR & 0xF8;
  return ret == I2C_DATA_ACK || ret == I2C_SLA_W_ACK || ret == I2C_SLA_R_ACK;
}

uint8_t i2c_read()
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))) ;
	return (TWDR);
}

void i2c_read(uint8_t* data, uint8_t count)
{
  while (count--) {
    if (count) TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    else       TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) ;
    *data++ = TWDR;
  }
}

void i2c_read_long(uint8_t* data, uint16_t count)
{
  while (count--) {
    if (count) TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    else       TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT))) ;
    *data++ = TWDR;
  }
}

bool i2c_begin_write(uint8_t addr)
{
  if (i2cStart()) {
    return i2c_write(addr<<1);
  } else
    return false;
}

bool i2c_begin_read(uint8_t addr)
{
  if (i2cStart()) {
    return i2c_write((addr<<1) | 1);
  } else
    return false; 
}

bool i2c_device_found(uint8_t addr)
{
  if (i2c_begin_write(addr)) {
    i2c_end();
    return true;
  } else
    return false;
}

// Init TWI (I2C)
void i2c_init()
{
  // внутренние pull-up резисторы
  digitalWrite(SCL, 1);
  digitalWrite(SDA, 1);
	TWBR = 92;						
	TWSR = 0;
	TWDR = 0xFF;
	PRR = 0;
}
