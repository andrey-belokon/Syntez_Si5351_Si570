#ifndef DISP_MAX7219_H
#define DISP_MAX7219_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "TRX.h"

class Display_MAX7219: public TRXDisplay {
  public:
	  Display_MAX7219(uint8_t _bright);
	  void setup();
	  void Draw(TRX& trx);
};

#endif
