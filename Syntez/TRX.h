#ifndef TRX_H
#define TRX_H

#include <Arduino.h>
#include "config.h"
#include "Eeprom24C32.h"

// состояние VFO для диапазона
typedef struct {
  long VFO[2];    // VFO freq A&B
  uint8_t  VFO_Index; // 0-A, 1-B
  uint8_t sideband;
  uint8_t AttPre;    // 0-nothing; 1-ATT; 2-Preamp
  uint8_t Split;
} TVFOState;

class TRX {
  public:
	   TVFOState BandData[BAND_COUNT+1]; // last - General coverage
    int SaveBandIndex;  // для правильного выхода из General coverage
    int BandIndex;  // -1 в режиме General coverage
    TVFOState state;
    uint8_t TX;
    uint8_t Lock;
    uint8_t RIT;
    int RIT_Value;
	  uint8_t QRP;
	  uint8_t SMeter; // 0..15 

	  TRX();
    void SwitchToBand(int band);
    void ExecCommand(uint8_t cmd);
    inline int GetVFOIndex() {
      return (state.Split && TX ? state.VFO_Index ^ 1 : state.VFO_Index);
    }
    void ChangeFreq(long freq_delta);
    
    uint16_t StateHash();
    void StateLoad(Eeprom24C32 &eep);
    void StateSave(Eeprom24C32 &eep);
};

class TRXDisplay {
  public:
    virtual void setup() {}
    virtual void reset() {}
	  virtual void Draw(TRX& trx) {}
    virtual void clear() {}
    virtual void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize);
};

#endif

