#include "Keypad_6_I2C.h"
#include "i2c.h"
#include "config.h"

void Keypad_6_I2C::setup() {
  if (i2c_device_found(i2c_addr))
    pcf8574_write(0xFF);
  else
    i2c_addr=0;
}

const uint8_t KeyMap[] = {
  cmdNone,
//*  вариант когда RIT вынесен на отдельную кнопку
  cmdBandDown,  cmdBandUp,  cmdAttPre,  cmdVFOSel,  cmdRIT,
  cmdHam,       cmdUSBLSB,  cmdQRP,     cmdSplit,   cmdZero,  // with Fn pressed
  cmdLock,      cmdMode,    cmdTune,    cmdVFOEQ,   cmdNone   // длинные нажатия
/*  вариант когда переключение моды вынесено на отдельную кнопку
  cmdBandDown,  cmdBandUp,  cmdAttPre,  cmdVFOSel,  cmdMode,
  cmdHam,       cmdZero,    cmdQRP,     cmdSplit,   cmdNone,  // with Fn pressed
  cmdLock,      cmdRIT,     cmdTune,    cmdVFOEQ,   cmdUSBLSB   // длинные нажатия
*/
};

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
      if (fn) code+=5;
      last_scan = code;
      last_code_tm = millis();
      code = KeyMap[code];
      last_code = code;
      KeyPressed = true;
      if (!fn && KeyMap[last_code+10] != cmdNone) {
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

void Keypad_6_I2C::pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
  i2c_write(data);
  i2c_end();
}

uint8_t Keypad_6_I2C::pcf8574_byte_read() 
{
  i2c_begin_read(i2c_addr);
  uint8_t data = i2c_read();
  i2c_end();
  return data;
}
