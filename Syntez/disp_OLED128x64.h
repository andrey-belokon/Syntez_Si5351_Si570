#ifndef DISP_OLED12864_H
#define DISP_OLED12864_H

#include <Arduino.h>
#include "TRX.h"

class Display_OLED128x64 {
  public:
	  void setup();
    // 1..15
    void setBright(uint8_t brightness);
	  void Draw(TRX& trx);
    void clear();
    void reset();
    void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize);
};

#endif
