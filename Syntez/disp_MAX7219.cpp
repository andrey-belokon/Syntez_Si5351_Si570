#include "disp_MAX7219.h"

#define DIN_PIN   11
#define CS_PIN    10
#define CLK_PIN   13

#define DECODEMODE_ADDR 9
#define BRIGHTNESS_ADDR  10
#define SCANLIMIT_ADDR  11
#define SHUTDOWN_ADDR 12
#define DISPLAYTEST_ADDR 15

uint8_t brightness;
long lastfreq;

uint8_t charTable [] = {
  B01111110,B00110000,B01101101,B01111001,B00110011,B01011011,B01011111,B01110000,B01111111,B01111011
};

void max7219_write(volatile byte address, volatile byte data) {
  digitalWrite(CS_PIN, LOW);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  digitalWrite(CS_PIN, HIGH);
}

void max7219_clear() {
  for (int i = 1; i <= 8; i++) {
    max7219_write(i, B00000000);
  }
}

 void max7219_setDigitLimit(uint8_t limit) {
  max7219_write(DISPLAYTEST_ADDR, 0);
  max7219_write(SCANLIMIT_ADDR, limit-1);
  // 0: Register Format
  // 255: Code B Font (0xff)
  max7219_write(DECODEMODE_ADDR, 0);
  max7219_clear();
  max7219_write(SHUTDOWN_ADDR, 1);
}

Display_MAX7219::Display_MAX7219(uint8_t _bright){
  brightness = _bright;
}

void Display_MAX7219::setup() 
{
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  max7219_write(BRIGHTNESS_ADDR, brightness);
  max7219_setDigitLimit(8);
  max7219_clear();
}

void Display_MAX7219::Draw(TRX& trx) 
{
  long freq = trx.state.VFO[trx.GetVFOIndex()];
  if (freq != lastfreq) {
    lastfreq = freq;
    for (uint8_t i=1; i <= 8; i++) {
      if (freq) {
        uint8_t ch = charTable[freq%10];
        if (i == 4 || i == 7) ch |= B10000000;
        max7219_write(i,ch);
        freq /= 10;
      } else {
        max7219_write(i,0);
      }
    }
  }
}

