#ifndef TRX_H
#define TRX_H

#include <Arduino.h>
#include "config.h"
#include "Eeprom24C32.h"

// состояние VFO для диапазона
typedef struct {
  long VFO[2];    // VFO freq A&B
  uint8_t  VFO_Index; // 0-A, 1-B
  uint8_t mode;
  uint8_t AttPre;    // 0-nothing; 1-ATT; 2-Preamp
  uint8_t Split;
} TVFOState;

class TRX {
  public:
	  static TVFOState BandData[]; // last - General coverage
    static int SaveBandIndex;  // для правильного выхода из General coverage
    static int BandIndex;  // -1 в режиме General coverage
    static TVFOState state;
    static uint8_t TX;
    static uint8_t CATTX;
    static uint8_t Lock;
    static uint8_t RIT;
    static int RIT_Value;
	  static uint8_t QRP;
    static uint8_t Tune;
	  static uint8_t SMeter; // 0..15 

	  TRX();
    static void SwitchToBand(int band);
    static void ExecCommand(uint8_t cmd);
    static uint8_t GetVFOIndex();
    static void ChangeFreq(long freq_delta);
    static void SetFreq(long freq, byte idx);

    static uint16_t StateHash();
    static void StateLoad(Eeprom24C32 &eep);
    static void StateSave(Eeprom24C32 &eep);

    static int IFreqShift[][3];
    static void IFreqShiftLoad(Eeprom24C32 &eep);
    static void IFreqShiftSave(Eeprom24C32 &eep);
};

class TRXDisplay {
  public:
    virtual void setup() = 0;
    virtual void reset() = 0;
	  virtual void Draw(TRX& trx) = 0;
    virtual void clear() = 0;
    virtual void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize) = 0;
    virtual void setBright(uint8_t brightness) {}
};

#endif
