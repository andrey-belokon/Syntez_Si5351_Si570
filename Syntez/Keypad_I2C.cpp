#include "Keypad_I2C.h"
#include "i2c.h"
#include "config.h"

void KeypadI2C::setup() {
  i2c_init();
  if (i2c_device_found(i2c_addr))
    pcf8574_write(0xFF);
  else
    i2c_addr=0;
}

uint8_t KeypadI2C::Read() 
{
  if (i2c_addr == 0) return cmdNone;
  if (millis()-last_code_tm < 50) return cmdNone;
  // read scan from PCF8574
  pcf8574_write(0xFF);
  uint8_t scan = ~pcf8574_byte_read();
  uint8_t code = 0;
  if (scan & 0x01) code = 1;
  else if (scan & 0x02) code = 2;
  else if (scan & 0x04) code = 3;
  else if (scan & 0x08) code = 4;
  else if (scan & 0x10) code = 5;
  else if (scan & 0x40) code = 6;

  if (code) {
    if (last_code) return cmdNone;
    else {
      if (scan & 0x20) code+=6;
      last_code = code;
      last_code_tm = millis();
      KeyPressed = true;
      return code;
    }
  } else if (scan & 0x20) {
    last_code = 0;
    FnPressed = true;
    fn_press_tm = millis();
    return cmdNone;
  } if (FnPressed && !KeyPressed) {
    if (millis()-fn_press_tm < 50) return cmdNone;
    FnPressed = false;
    KeyPressed = false;
    last_code = 0;
    last_code_tm = millis();
    return cmdMenu;
  } else {
    FnPressed = false;
    KeyPressed = false;
    last_code = 0;
    return cmdNone;
  }
}

void KeypadI2C::pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
  i2c_write(data);
  i2c_end();
}

uint8_t KeypadI2C::pcf8574_byte_read() 
{
  i2c_begin_read(i2c_addr);
  uint8_t data = i2c_read();
  i2c_end();
  return data;
}
