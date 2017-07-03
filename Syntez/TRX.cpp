#include "TRX.h"

const struct _Bands Bands[BAND_COUNT] = {
  DEFINED_BANDS
};

TRX::TRX() {
  for (int i=0; i < BAND_COUNT; i++) {
	BandData[i].VFO_Index = 0;
	if (Bands[i].startSSB != 0)
	  BandData[i].VFO[0] = BandData[i].VFO[1] = Bands[i].startSSB;
	else
	  BandData[i].VFO[0] = BandData[i].VFO[1] = Bands[i].start;
	BandData[i].sideband = Bands[i].sideband;
	BandData[i].AttPre = 0;
	BandData[i].Split = false;
  }
  Lock = RIT = TX = QRP = false;
  RIT_Value = 0;
}

void TRX::SwitchToBand(int band) {
  BandIndex = band;
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
    } else {
      if (state.VFO[state.VFO_Index] < FREQ_MIN)
        state.VFO[state.VFO_Index] = FREQ_MIN;
      else if (state.VFO[state.VFO_Index] > FREQ_MAX)
        state.VFO[state.VFO_Index] = FREQ_MAX;
    }
  }
}

void TRX::ExecCommand(uint8_t cmd) {
  if (TX && (cmd != cmdQRP)) return; // блокируем при передаче
  switch (cmd) {
    case cmdAttPre: // переключает по кругу аттенюатор/увч
      if (++state.AttPre > 2) state.AttPre = 0;
      break;
    case cmdVFOSel: // VFO A/B
      state.VFO_Index ^= 1;
      break;
    case cmdUSBLSB:
      state.sideband = (state.sideband == LSB ? USB : LSB);
      break;
    case cmdVFOEQ:
      state.VFO[state.VFO_Index ^ 1] = state.VFO[state.VFO_Index];
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
      } else {
        if ((state.VFO[state.VFO_Index]+=1000000) > FREQ_MAX)
          state.VFO[state.VFO_Index] = FREQ_MAX;
      }
      break;
    case cmdBandDown:
      if (BandIndex >= 0) {
        memcpy(&BandData[BandIndex],&state,sizeof(TVFOState));
		    if (--BandIndex < 0)
          BandIndex = BAND_COUNT-1;
        memcpy(&state,&BandData[BandIndex],sizeof(TVFOState));
        Lock = RIT = false;
      } else {
        if ((state.VFO[state.VFO_Index]-=1000000) < FREQ_MIN)
          state.VFO[state.VFO_Index] = FREQ_MIN;
      }
      break;
    case cmdHam:
      if (BandIndex >= 0)
        BandIndex = -1;
      else {
        long min_delta = FREQ_MAX;
        int min_delta_idx = -1;
        for (int i=0; i < BAND_COUNT; i++) {
          long delta = abs(state.VFO[state.VFO_Index]-Bands[i].start);
          if (delta < min_delta) {
            min_delta = delta;
            min_delta_idx = i;
          }
        }
        BandIndex = min_delta_idx;
      }
      break;
  }
}

