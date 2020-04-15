#ifndef DISP_1602_H
#define DISP_1602_H

#include <Arduino.h>
#include "TRX.h"
#include "LCD1602_I2C.h"

class Display_1602_I2C: public TRXDisplay {
  private:
  	LiquidCrystal_I2C lcd;
    uint8_t tx;
  public:
	  Display_1602_I2C (int i2c_addr): lcd(i2c_addr,16,2) {}
	  void setup();
    void reset() {}
	  void Draw(TRX& trx);
    void clear() { lcd.clear(); }
    void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize);
};

#endif
