#include "TRX.h"

const struct _Bands Bands[] = {
  DEFINED_BANDS
};

const struct _Modes Modes[] = {
  DEFINED_MODES ,
  { NULL, 0, 0, 0, 0, {0, 0} }
};

#define BAND_COUNT  ((int)(sizeof(Bands)/sizeof(struct _Bands)))
#define MODE_COUNT  ((int)(sizeof(Modes)/sizeof(struct _Modes)-1))

int TRX::IFreqShift[MODE_COUNT][3];
TVFOState TRX::BandData[BAND_COUNT+1];
int TRX::SaveBandIndex;
int TRX::BandIndex;  // -1 в режиме General coverage
TVFOState TRX::state;
uint8_t TRX::TX;
uint8_t TRX::Lock;
uint8_t TRX::RIT;
int TRX::RIT_Value;
uint8_t TRX::QRP;
uint8_t TRX::Tune;
uint8_t TRX::SMeter; // 0..15 

TRX::TRX() {
  memset(IFreqShift,0,sizeof(IFreqShift));
  for (byte i=0; i < BAND_COUNT; i++) {
	  BandData[i].VFO_Index = 0;
    BandData[i].VFO[0] = BandData[i].VFO[1] = ((Bands[i].start+Bands[i].end)/2000)*1000;
    BandData[i].mode = Bands[i].mode;
	  BandData[i].AttPre = 0;
	  BandData[i].Split = false;
  }
  Lock = RIT = TX = QRP = false;
  RIT_Value = 0;
  BandData[BAND_COUNT].VFO[0] = 0;
  SwitchToBand(BAND_COUNT == 1 ? 0 : 1);
}

uint8_t TRX::GetVFOIndex() {
  return (state.Split && TX ? state.VFO_Index ^ 1 : state.VFO_Index);
}
    
uint16_t hash_data(uint16_t hval, uint8_t* data, int sz)
{
  while (sz--) {
    hval ^= *data++;
    hval = (hval << 11) | (hval >> 5);
  }
  return hval;
}  

uint16_t TRX::StateHash() {
  uint16_t hval = 5381;
  hval = hash_data(hval, (uint8_t*)BandData, sizeof(BandData));
  hval = hash_data(hval, (uint8_t*)&BandIndex, sizeof(BandIndex));
  hval = hash_data(hval, (uint8_t*)&state, sizeof(state));
  return hval;
}

#define CRC_SIGN          0x579E
#define STATE_ADDR        0
#define IFreqShift_ADDR   (sizeof(sign)+sizeof(BandData)+sizeof(SaveBandIndex)+sizeof(BandIndex)+sizeof(state))

void TRX::StateLoad(Eeprom24C32 &eep) {
  uint16_t sign=0,addr=STATE_ADDR;
  eep.readBytes(addr,sizeof(sign),(byte*)&sign);
  if (sign == (CRC_SIGN^IFreqShift_ADDR)) { // sign xor size
    addr += sizeof(sign);
    eep.readBytes(addr,sizeof(BandData),(byte*)BandData);
    addr += sizeof(BandData);   
    eep.readBytes(addr,sizeof(SaveBandIndex),(byte*)&SaveBandIndex);
    addr += sizeof(SaveBandIndex);
    eep.readBytes(addr,sizeof(BandIndex),(byte*)&BandIndex);
    addr += sizeof(BandIndex);
    eep.readBytes(addr,sizeof(state),(byte*)&state);
  }
}

void TRX::StateSave(Eeprom24C32 &eep) {
  uint16_t sign=CRC_SIGN^IFreqShift_ADDR, addr=STATE_ADDR;
  eep.writeBytes(addr,sizeof(sign),(byte*)&sign);
  addr += sizeof(sign);
  eep.writeBytes(addr,sizeof(BandData),(byte*)BandData);
  addr += sizeof(BandData);
  eep.writeBytes(addr,sizeof(SaveBandIndex),(byte*)&SaveBandIndex);
  addr += sizeof(SaveBandIndex);
  eep.writeBytes(addr,sizeof(BandIndex),(byte*)&BandIndex);
  addr += sizeof(BandIndex);
  eep.writeBytes(addr,sizeof(state),(byte*)&state);
}

void TRX::IFreqShiftLoad(Eeprom24C32 &eep)
{
  uint16_t sign=0,addr=IFreqShift_ADDR;
  eep.readBytes(addr,sizeof(sign),(byte*)&sign);
  if (sign == (CRC_SIGN^sizeof(IFreqShift))) {
    addr += sizeof(sign);
    eep.readBytes(addr,sizeof(IFreqShift),(byte*)IFreqShift);
  }
}

void TRX::IFreqShiftSave(Eeprom24C32 &eep)
{
  uint16_t sign=CRC_SIGN^sizeof(IFreqShift), addr=IFreqShift_ADDR;
  eep.writeBytes(addr,sizeof(sign),(byte*)&sign);
  addr += sizeof(sign);
  eep.writeBytes(addr,sizeof(IFreqShift),(byte*)IFreqShift);
}

void TRX::SwitchToBand(int band) {
  SaveBandIndex = BandIndex = band;
  memcpy(&state,&BandData[BandIndex],sizeof(TVFOState));
  Lock = RIT = false;
  RIT_Value = 0;
}

void TRX::ChangeFreq(long freq_delta) {
  if (!TX && !Lock) {
    state.VFO[state.VFO_Index] += freq_delta;
    // проверяем выход за пределы диапазона
    if (BandIndex >= 0) {
      if (state.VFO[state.VFO_Index] < Bands[BandIndex].start)
        state.VFO[state.VFO_Index] = Bands[BandIndex].start;
      else if (state.VFO[state.VFO_Index] > Bands[BandIndex].end)
        state.VFO[state.VFO_Index] = Bands[BandIndex].end;
#ifdef GENERAL_COVERAGE_ENABLED
    } else {
      if (state.VFO[state.VFO_Index] < FREQ_MIN)
        state.VFO[state.VFO_Index] = FREQ_MIN;
      else if (state.VFO[state.VFO_Index] > FREQ_MAX)
        state.VFO[state.VFO_Index] = FREQ_MAX;
#endif
    }
    RIT = false;
  }
}

void TRX::ExecCommand(uint8_t cmd) {
  if (TX && (cmd != cmdQRP) && (cmd != cmdTune)) return; // блокируем при передаче
  switch (cmd) {
    case cmdAttPre: // переключает по кругу аттенюатор/увч
      if (++state.AttPre > 2) state.AttPre = 0;
      break;
    case cmdVFOSel: // VFO A/B
      state.VFO_Index ^= 1;
      break;
    case cmdMode:
      if (Modes[++state.mode].name == NULL) state.mode = 0; // по кругу
      if (!Modes[state.mode].tx_enable) Tune = 0;
      break;
    case cmdVFOEQ:
      state.VFO[state.VFO_Index ^ 1] = state.VFO[state.VFO_Index];
      break;
    case cmdTune:
      if (!Modes[state.mode].tx_enable) Tune = 0;
      else Tune = !Tune;
      break;
    case cmdQRP:
      QRP = !QRP;
      break;
    case cmdLock:
      Lock = !Lock;
      break;
    case cmdSplit:
      state.Split = !state.Split;
      break;
    case cmdZero:
      state.VFO[state.VFO_Index] = ((state.VFO[state.VFO_Index]+500) / 1000)*1000;
      break;
    case cmdRIT:
      RIT = !RIT;
      break;
    case cmdBandUp:
	    if (BandIndex >= 0) {
		    memcpy(&BandData[BandIndex],&state,sizeof(TVFOState));
        if (++BandIndex >= BAND_COUNT)
          BandIndex = 0;
        memcpy(&state,&BandData[BandIndex],sizeof(TVFOState));
        Lock = RIT = false;
#ifdef GENERAL_COVERAGE_ENABLED
      } else {
        if ((state.VFO[state.VFO_Index]+=1000000) > FREQ_MAX)
          state.VFO[state.VFO_Index] = FREQ_MAX;
        Lock = false;
#endif
      }
      break;
    case cmdBandDown:
      if (BandIndex >= 0) {
        memcpy(&BandData[BandIndex],&state,sizeof(TVFOState));
		    if (--BandIndex < 0)
          BandIndex = BAND_COUNT-1;
        memcpy(&state,&BandData[BandIndex],sizeof(TVFOState));
        Lock = RIT = false;
#ifdef GENERAL_COVERAGE_ENABLED
      } else {
        if ((state.VFO[state.VFO_Index]-=1000000) < FREQ_MIN)
          state.VFO[state.VFO_Index] = FREQ_MIN;
        Lock = false;
#endif
      }
      break;
#ifdef GENERAL_COVERAGE_ENABLED
    case cmdHam:
      if (BandIndex >= 0) {
        memcpy(&BandData[BandIndex],&state,sizeof(TVFOState));
        SaveBandIndex = BandIndex;
        BandIndex = -1;
        if (BandData[BAND_COUNT].VFO[0] != 0)
          memcpy(&state,&BandData[BAND_COUNT],sizeof(TVFOState));
      } else {
        memcpy(&BandData[BAND_COUNT],&state,sizeof(TVFOState));
        BandIndex = SaveBandIndex;
        memcpy(&state,&BandData[BandIndex],sizeof(TVFOState));
      }
      Lock = RIT = false;
      break;
#endif
  }
}
