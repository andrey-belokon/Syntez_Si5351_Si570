#include "Keypad_12_I2C.h"
#include "i2c.h"
#include "config.h"

// мапинг сканкодов на команды для клавиатуры 3x4
const uint8_t KeyMap[4][4] = {
  cmdBandUp, cmdBandDown, cmdAttPre, cmdVFOSel,
  cmdVFOEQ,  cmdUSBLSB,   cmdMenu,   cmdSplit,
  cmdRIT,    cmdHam,      cmdZero,   cmdQRP,
  cmdNone,   cmdNone,     cmdNone,   cmdNone
};

void Keypad_12_I2C::setup() {
  if (i2c_device_found(i2c_addr))
    pcf8574_write(0xFF);
  else
    i2c_addr=0;
}

uint8_t Keypad_12_I2C::read_scan() 
{
  if (i2c_addr) {
    pcf8574_write(0xFF);
    for (byte row=0; row <= 2; row++) {
      pcf8574_write(~(1<<row));
      switch (~(pcf8574_byte_read() >> 4) & 0xF) {
        case 0x1: return row;
        case 0x2: return 0x10+row;
        case 0x4: return 0x20+row;
        case 0x8: return 0x30+row;
      }
    }
  }
  return 0xFF;
}

uint8_t Keypad_12_I2C::Read() 
{
  if (i2c_addr == 0) return cmdNone;
  if (millis()-last_code_tm < 50) return cmdNone;
  uint8_t code = read_scan(); 
  code = KeyMap[code & 0xF][code >> 4];
  if (code == last_code) return cmdNone;
  last_code = code;
  last_code_tm = millis();
  return code;
}

void Keypad_12_I2C::pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
  i2c_write(data);
  i2c_end();
}

uint8_t Keypad_12_I2C::pcf8574_byte_read() 
{
  i2c_begin_read(i2c_addr);
  uint8_t data = i2c_read();
  i2c_end();
  return data;
}
