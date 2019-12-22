#include "config_hw.h"
#include "RTC.h"
#include "i2c.h"

#ifdef RTC_PCF8563
#define RTC_I2C_ADDRESS 0x51
#define REG_ADDR        0x2
#endif

#ifdef RTC_DS1307
#define RTC_I2C_ADDRESS 0x68
#define REG_ADDR        0
#endif

//const uint8_t RTC_BITMASK[7] = {B01111111, B01111111, B00111111, B00111111, B00000111, B00011111, 0xFF};
 
void RTC_Write(RTCData* data)
{
  byte *p=(byte *)data;
  if (i2c_begin_write(RTC_I2C_ADDRESS)) {
    i2c_write(REG_ADDR);
    for (byte i=7; i > 0; i--) i2c_write(*p++);
    i2c_end();
  }
}

void RTC_Read(RTCData* data)
{
  if (i2c_begin_write(RTC_I2C_ADDRESS)) {
    i2c_write(REG_ADDR);
    i2c_begin_read(RTC_I2C_ADDRESS);
    i2c_read((uint8_t*)data,sizeof(RTCData));
    i2c_end();
    //uint8_t *p = (uint8_t*)data;
    //for (byte i=0; i < 7; i++) *p++ &= RTC_BITMASK[i];
  }
}
