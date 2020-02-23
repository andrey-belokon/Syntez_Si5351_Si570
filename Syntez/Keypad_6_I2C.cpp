#include "Keypad_6_I2C.h"
#include <i2c.h>
#include "config.h"

const uint8_t KeyMap[] = {
  cmdNone,
//*  вариант когда RIT вынесен на отдельную кнопку
  cmdBandDown,  cmdBandUp,  cmdAttPre,  cmdVFOSel,  cmdRIT,   // одиночное нажатие
  cmdHam,       cmdNone,    cmdQRP,     cmdSplit,   cmdZero,  // with Fn pressed
  cmdLock,      cmdMode,    cmdTune,    cmdVFOEQ,   cmdNone   // длинные нажатия
};

uint8_t i2c_addr;
uint8_t Keypad_6_I2C::last_code = -1;
uint8_t last_scan = 0;
uint8_t Keypad_6_I2C::FnPressed = 0, Keypad_6_I2C::KeyPressed = 0;
uint8_t longpress = 0;
long last_code_tm = 0;
long fn_press_tm = 0;

void pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
  i2c_write(data);
  i2c_end();
}

uint8_t pcf8574_byte_read() 
{
  i2c_begin_read(i2c_addr);
  uint8_t data = i2c_read();
  i2c_end();
  return data;
}

Keypad_6_I2C::Keypad_6_I2C(uint8_t _i2c_addr)
{
  i2c_addr = _i2c_addr;
}

void Keypad_6_I2C::setup() {
  if (i2c_device_found(i2c_addr))
    pcf8574_write(0xFF);
  else
    i2c_addr=0;
}

uint8_t Keypad_6_I2C::Read() 
{
  if (i2c_addr == 0) return cmdNone;
  if (millis()-last_code_tm < 50) return cmdNone;
  // read scan from PCF8574
  pcf8574_write(0xFF);
  uint8_t scan = ~pcf8574_byte_read();
  uint8_t code = 0;
  uint8_t fn = scan & 0x20;
  if (scan & 0x01) code = 1;
  else if (scan & 0x02) code = 2;
  else if (scan & 0x04) code = 3;
  else if (scan & 0x08) code = 4;
  else if (scan & 0x10) code = 5;

  if (code) {
    if (last_scan) {
      if (longpress && millis()-last_code_tm > 1000) {
        longpress = 0;
        last_code = KeyMap[last_scan+10];
        return last_code;
      }
      return cmdNone;
    } else {
      last_scan = code;
      last_code_tm = millis();
      if (fn) code+=5;
      code = KeyMap[code];
      last_code = code;
      KeyPressed = true;
      if (!fn && KeyMap[last_scan+10] != cmdNone) {
        longpress = 1;
        code = cmdNone; // long press
      }
      return code;
    }
  } else if (fn) {
    last_code = 0;
    last_scan = 0;
    FnPressed = true;
    fn_press_tm = millis();
    return cmdNone;
  } if (FnPressed && !KeyPressed) {
    if (millis()-fn_press_tm < 50) return cmdNone;
    FnPressed = false;
    KeyPressed = false;
    last_code = 0;
    last_scan = 0;
    last_code_tm = millis();
    return cmdMenu;
  } else {
    FnPressed = false;
    KeyPressed = false;
    if (longpress) {
      longpress = 0;
      return KeyMap[last_scan];
    }
    last_code = 0;
    last_scan = 0;
    return cmdNone;
  }
}
