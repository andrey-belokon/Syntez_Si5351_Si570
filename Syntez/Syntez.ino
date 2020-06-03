//
// UR5FFR Si570/Si5351 VFO
// v3.1 from 15.04.2020
// Copyright (c) Andrey Belokon, 2016-2020 Odessa 
// https://github.com/andrey-belokon/
// GNU GPL license
//

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! all user setting defined in config.h, config_hw.h and config_sw.h files !!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "config.h"

#include <avr/eeprom.h> 
#include "utils.h"
#include <i2c.h>
#include "pins.h"

#ifdef ENCODER_ENABLE
  #include "Encoder.h"
#endif

#ifdef KEYPAD_6
  #include "Keypad_6_I2C.h"
#endif
#ifdef KEYPAD_7
  #include "Keypad_7_I2C.h"
#endif
#ifdef KEYPAD_12
  #include "Keypad_12_I2C.h"
#endif

#ifdef RTC_ENABLE
  #include "RTC.h"
#endif

#include "Eeprom24C32.h"
#include "TRX.h"

#ifdef DISPLAY_ILI9341
  #include "disp_ILI9341.h"
#endif
#ifdef DISPLAY_ST7735
  #include "disp_ST7735.h"
#endif
#ifdef DISPLAY_1602
  #include "disp_1602.h"
#endif
#ifdef DISPLAY_OLED128x32
  #include "disp_OLED128x32.h"
#endif
#ifdef DISPLAY_OLED128x64
  #include "disp_OLED128x64.h"
#endif
#ifdef DISPLAY_OLED_SH1106_128x64
  #include "disp_OLED128x64.h"
#endif

#if defined(VFO_SI5351) || defined(VFO_SI5351_2)
  #include <si5351a.h>
#endif
#ifdef VFO_SI570  
  #include <Si570.h>
#endif

#define KEYPAD_DISABLE
#ifdef KEYPAD_6
  Keypad_6_I2C keypad(KEYPAD_6);
  #undef KEYPAD_DISABLE
#endif
#ifdef KEYPAD_7
  Keypad_7_I2C keypad(KEYPAD_7);
  #undef KEYPAD_DISABLE
#endif
#ifdef KEYPAD_12
  Keypad_12_I2C keypad(KEYPAD_12);
  #undef KEYPAD_DISABLE
#endif

#define DISPLAY_DISABLE
#ifdef DISPLAY_1602
  Display_1602_I2C disp(I2C_ADR_DISPLAY_1602);
  #undef DISPLAY_DISABLE
#endif
#ifdef DISPLAY_ILI9341
  Display_ILI9341_SPI disp;
  #undef DISPLAY_DISABLE
#endif
#ifdef DISPLAY_ST7735
  Display_ST7735_SPI disp;
  #undef DISPLAY_DISABLE
#endif
#ifdef DISPLAY_OLED128x32
  Display_OLED128x32 disp;
  #undef DISPLAY_DISABLE
#endif
#ifdef DISPLAY_OLED128x64
  Display_OLED128x64 disp;
  #undef DISPLAY_DISABLE
#endif
#ifdef DISPLAY_OLED_SH1106_128x64
  Display_OLED128x64 disp;
  #undef DISPLAY_DISABLE
#endif

TRX trx;

Eeprom24C32 ee24c32(I2C_ADR_EE24C32);

#ifdef VFO_SI5351
  Si5351 vfo5351;
#endif
#ifdef VFO_SI5351_2
  Si5351 vfo5351_2;
#endif
#ifdef VFO_SI570  
  Si570 vfo570;
#endif

#if defined(VFO_SI5351) && defined(VFO_SI5351_2)
  #define   SELECT_SI5351(x)    digitalWrite(PIN_SELECT_SI5351,(x))
#else
  #define   SELECT_SI5351(x)
#endif

int EEMEM SMeterMap_EEMEM[15] = {0};
int SMeterMap[15];

InputPullUpPin inTX(PIN_IN_TX);
InputPullUpPin inTune(PIN_IN_TUNE);
OutputBinPin outTX(PIN_OUT_TX,false,HIGH);
OutputBinPin outQRP(PIN_OUT_QRP,false,HIGH);
#ifdef PIN_IN_RIT
InputAnalogPin inRIT(PIN_IN_RIT,5);
#endif
InputAnalogPin inSMeter(PIN_IN_SMETER);

#ifdef BANDCTRL_ENABLE
  OutputPCF8574 outBandCtrl(I2C_ADR_BAND_CTRL,0);
#endif

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup()
{
  // set an ADC prescale to 16 --> 76.8kHz per sample
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  i2c_init(400000);
#ifdef PIN_SELECT_SI5351
  pinMode(PIN_SELECT_SI5351,OUTPUT);
  digitalWrite(PIN_SELECT_SI5351,0);
#endif
  eeprom_read_block(SMeterMap, SMeterMap_EEMEM, sizeof(SMeterMap));
#ifdef CAT_ENABLE
  Serial.begin(COM_BAUND_RATE);
#endif    
#ifdef VFO_SI5351
  // change for required output level
  SELECT_SI5351(0);
  vfo5351.setup(
    SI5351_CLK0_DRIVE,
    SI5351_CLK1_DRIVE,
    SI5351_CLK2_DRIVE
  );
  vfo5351.set_xtal_freq(SI5351_CALIBRATION);
#endif  
#ifdef VFO_SI5351_2
  // change for required output level
  SELECT_SI5351(1);
  vfo5351_2.setup(
    SI5351_2_CLK0_DRIVE,
    SI5351_2_CLK1_DRIVE,
    SI5351_2_CLK2_DRIVE
  );
  vfo5351_2.set_xtal_freq(SI5351_2_CALIBRATION);
#endif  
#ifdef VFO_SI570  
  vfo570.setup(SI570_CALIBRATION);
#endif  
#ifdef ENCODER_ENABLE
  Encoder::Setup();
#endif  
#ifndef KEYPAD_DISABLE
  keypad.setup();
#endif
  inTX.setup();
  inTune.setup();
#ifdef PIN_IN_RIT
  inRIT.setup();
#endif
  outTX.setup();
  outQRP.setup();
#ifndef DISPLAY_DISABLE
  disp.setup();
#endif
  if (ee24c32.found()) {
    ee24c32.setup();
    trx.IFreqShiftLoad(ee24c32);
    trx.StateLoad(ee24c32);
  }
}

#define FQGRAN    10
long last_f1=0, last_f2=0, last_f3=0;

void vfo_set_freq(long f1, long f2, long f3)
{
/*  if (f1 != last_f1 || f2 != last_f2 || f3 != last_f3) {
    // debug
    Serial.print(f1); Serial.print("   ");
    Serial.print(f2); Serial.println();
  } */
  f1 = ((f1+FQGRAN/2)/FQGRAN)*FQGRAN;
  f2 = ((f2+FQGRAN/2)/FQGRAN)*FQGRAN;
  f3 = ((f3+FQGRAN/2)/FQGRAN)*FQGRAN; 
#ifdef VFO_SI570  
  vfo570.set_freq(f1);
  #ifdef VFO_SI5351_2
    if (last_f2 != f2) {
      SELECT_SI5351(0);
      vfo5351.set_freq(f2);
      last_f2 = f2;
    }
    if (last_f3 != f3) {
      SELECT_SI5351(1);
      vfo5351_2.set_freq(f3);
      last_f3 = f3;
    }
  #else
    #ifdef VFO_SI5351
      vfo5351.set_freq(f2,f3);
    #endif
  #endif
#else
  #ifdef VFO_SI5351_2
    if (last_f1 != f1) {
      SELECT_SI5351(0);
      vfo5351.set_freq(f1);
      last_f1 = f1;
    }
    if (last_f2 != f2 || last_f3 != f3) {
      SELECT_SI5351(1);
      vfo5351_2.set_freq(f2,f3);
      last_f2 = f2;
      last_f3 = f3;
    }
  #else
    #ifdef VFO_SI5351
      vfo5351.set_freq(f1,f2,f3);
    #endif
  #endif
#endif
}

void vfo_set_freq(long f1, long f2)
{
  f1 = (f1/FQGRAN)*FQGRAN;
  f2 = (f2/FQGRAN)*FQGRAN;
#ifdef VFO_SI570  
  vfo570.set_freq(f1);
  #ifdef VFO_SI5351_2
    if (last_f2 != f2) {
      SELECT_SI5351(0);
      vfo5351.set_freq(f2);
      last_f2 = f2;
    }
  #else
    #ifdef VFO_SI5351
      vfo5351.set_freq(f2);
    #endif
  #endif
#else
  #ifdef VFO_SI5351_2
    if (last_f1 != f1) {
      SELECT_SI5351(0);
      vfo5351.set_freq(f1);
      last_f1 = f1;
    }
    if (last_f2 != f2) {
      SELECT_SI5351(1);
      vfo5351_2.set_freq(f2);
      last_f2 = f2;
    }
  #else
    #ifdef VFO_SI5351
      vfo5351.set_freq(f1,f2);
    #endif
  #endif
#endif
}

void UpdateFreq() 
{

#ifdef MODE_DC
  vfo_set_freq(
    CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
    0
  );
#endif

#ifdef MODE_DC_QUADRATURE
  #ifdef VFO_SI5351
    SELECT_SI5351(0);
    vfo5351.set_freq_quadrature(
      trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0),
      0
    );
  #else
    #error MODE_DC_QUADRATURE allow only with installed Si5351
  #endif
#endif

#if defined(MODE_SINGLE_IF) || defined(MODE_SINGLE_IF_RXTX) || defined(MODE_SINGLE_IF_SWAP)
  const struct _Modes *mm = &Modes[trx.state.mode];
  int *freq_shift = trx.IFreqShift[trx.state.mode];
  long SSBDetectorFreq_LSB = mm->freq[0]+freq_shift[0];
  int rit_val = (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
  int shift_rx = (trx.TX ? 0 : mm->rx_shift+freq_shift[2]);
  uint8_t sbm = mm->sb_mode;
  if (sbm == SBM_DSB) {
    // no sideband. use freq as filter center freq
    vfo_set_freq( // гетеродин сверху
      CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + SSBDetectorFreq_LSB + rit_val),
      0
    );
  } else {
    long SSBDetectorFreq_USB = mm->freq[1]+freq_shift[1];
    long vfo,bfo;
    if (SSBDetectorFreq_LSB != 0 && SSBDetectorFreq_USB != 0) {
      // инверсия боковой - гетеродин сверху
      bfo = ((sbm == SBM_LSB) ? SSBDetectorFreq_USB : SSBDetectorFreq_LSB);
      vfo = trx.state.VFO[trx.GetVFOIndex()] + bfo + rit_val;
      bfo += ((sbm == SBM_LSB) ? -shift_rx : shift_rx);
    } else if (SSBDetectorFreq_USB != 0) {
      vfo = trx.state.VFO[trx.GetVFOIndex()] + rit_val;
      if (sbm == SBM_LSB) {
        vfo += SSBDetectorFreq_USB;
      } else {
        vfo = SSBDetectorFreq_USB-vfo;
        if (vfo < 0) vfo = -vfo;
      }
      bfo = SSBDetectorFreq_USB-shift_rx;
    } else {
      vfo = trx.state.VFO[trx.GetVFOIndex()] + rit_val;
      if (sbm == SBM_USB) {
        vfo += SSBDetectorFreq_LSB;
      } else {
        vfo = SSBDetectorFreq_LSB-vfo;
        if (vfo < 0) vfo = -vfo;
      }
      bfo = SSBDetectorFreq_LSB+shift_rx;
    }
    #ifdef MODE_SINGLE_IF
      vfo_set_freq(CLK0_MULT*vfo, CLK1_MULT*bfo);
    #endif
    #ifdef MODE_SINGLE_IF_RXTX
      vfo_set_freq(CLK0_MULT*vfo, 
        trx.TX ? 0 : CLK1_MULT*bfo,
        trx.TX ? CLK1_MULT*bfo : 0
      );
    #endif
    #ifdef MODE_SINGLE_IF_SWAP
      vfo_set_freq(
        trx.TX ? CLK1_MULT*bfo : CLK0_MULT*vfo,
        trx.TX ? CLK0_MULT*vfo : CLK1_MULT*bfo
      );
    #endif
  }
#endif

#if defined(MODE_DOUBLE_IF_SWAP23) || defined(MODE_DOUBLE_IF)

  const struct _Modes *mm = &Modes[trx.state.mode];
  int *freq_shift = trx.IFreqShift[trx.state.mode];
  long SSBDetectorFreq_LSB = mm->freq[0]+freq_shift[0];
  int rit_val = (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
  int shift_rx = (trx.TX ? 0 : mm->rx_shift+freq_shift[2]);
  uint8_t sbm = mm->sb_mode;
  if (sbm == SBM_DSB) {
    // no sideband. use LSB freq as filter center freq
    #ifndef IFreqEx
      long IFreqEx = (IFreqEx_LSB+IFreqEx_USB) >> 1;
    #endif
    vfo_set_freq( // гетеродин сверху
      CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + rit_val),
      CLK1_MULT*(IFreqEx + SSBDetectorFreq_LSB)
    );
  } else {
    #ifndef IFreqEx
      long IFreqEx = (sbm == SBM_USB ? IFreqEx_LSB : IFreqEx_USB);
    #endif
    long SSBDetectorFreq_USB = mm->freq[1]+freq_shift[1];
    long f1,f2,f3;
    if (SSBDetectorFreq_LSB != 0 && SSBDetectorFreq_USB != 0) {
      long IFreq = (sbm == SBM_USB ? SSBDetectorFreq_USB : SSBDetectorFreq_LSB); 
      f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + rit_val);
      f2 = CLK1_MULT*(IFreqEx + IFreq);
      f3 = CLK2_MULT*(IFreq + ((sbm == SBM_USB ? -shift_rx : shift_rx)));
    } else if (SSBDetectorFreq_LSB != 0) {
      f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + rit_val);
      f2 = CLK1_MULT*(IFreqEx + (sbm == SBM_LSB ? SSBDetectorFreq_LSB : -(SSBDetectorFreq_LSB)));
      f3 = CLK2_MULT*(SSBDetectorFreq_LSB + shift_rx);
    } else if (SSBDetectorFreq_USB != 0) {
      f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + rit_val);
      f2 = CLK1_MULT*(IFreqEx + (sbm == SBM_USB ? SSBDetectorFreq_USB : -(SSBDetectorFreq_USB)));
      f3 = CLK2_MULT*(SSBDetectorFreq_USB - shift_rx);
    }
    #if defined(MODE_DOUBLE_IF_SWAP23)
      vfo_set_freq(f1,
        trx.TX ? f3 : f2,
        trx.TX ? f2 : f3
      );
    #else
      vfo_set_freq(f1,f2,f3);
    #endif
  }

#endif
}

void UpdateBandCtrl() 
{
#ifdef BANDCTRL_ENABLE
  outBandCtrl.Set(BCPN_BAND_0, trx.BandIndex & 0x1);
  outBandCtrl.Set(BCPN_BAND_1, trx.BandIndex & 0x2);
  outBandCtrl.Set(BCPN_BAND_2, trx.BandIndex & 0x4);
  outBandCtrl.Set(BCPN_BAND_3, trx.BandIndex & 0x8);
  // 0-nothing; 1-ATT; 2-Preamp
  switch (trx.state.AttPre) {
    case 0:
      outBandCtrl.Set(BCPN_ATT,false);
      outBandCtrl.Set(BCPN_PRE,false);
      break;
    case 1:
      outBandCtrl.Set(BCPN_ATT,true);
      outBandCtrl.Set(BCPN_PRE,false);
      break;
    case 2:
      outBandCtrl.Set(BCPN_ATT,false);
      outBandCtrl.Set(BCPN_PRE,true);
      break;
  }
  outBandCtrl.Set(BCPN_CW, trx.state.mode == MODE_CW);
  outBandCtrl.Set(BCPN_SB, trx.state.mode == MODE_USB);
  outBandCtrl.Write();
#endif
}

#ifdef CAT_ENABLE
  #include "CAT.h"
#endif    

#ifndef KEYPAD_DISABLE
  #include "menu.h"
#endif    

void loop()
{
#ifndef KEYPAD_DISABLE
  uint8_t cmd;
  static long delay_cmd_tm = 0;
  cmd = keypad.Read();
  if (cmd != cmdNone && trx.Tune) {
    trx.Tune = 0;
    cmd = cmdNone;
  }
  if (trx.Lock && (cmd == cmdBandUp || cmd == cmdBandDown)) {
    trx.Lock = 0;
    cmd = cmdNone;
  }
  if (cmd != cmdNone) {
    if (cmd == cmdMenu) {
#ifndef DISPLAY_DISABLE
      // double press at 1sec
      if (millis()-delay_cmd_tm <= 1000) {
        // call to menu
        ShowMenu();
        // перерисовываем дисплей
        disp.clear();
        disp.reset();
        disp.Draw(trx);
        delay_cmd_tm = 0;
        return;
      } else {
        delay_cmd_tm = millis();
      }
#endif  
    } else {
      trx.ExecCommand(cmd);
    }
  }
#endif    

#ifdef ENCODER_ENABLE
  long delta = Encoder::GetDelta();
  if (delta) {
#ifdef KEYPAD_6
    if (keypad.IsKeyPressed() && keypad.GetLastCode() == cmdRIT) {  
      trx.RIT_Value = trx.RIT_Value + delta/10;
      if (trx.RIT_Value > RIT_MAX_VALUE) trx.RIT_Value = RIT_MAX_VALUE;
      if (trx.RIT_Value < -RIT_MAX_VALUE) trx.RIT_Value = -RIT_MAX_VALUE;
    } else {
#endif
#ifndef KEYPAD_DISABLE
    if (keypad.IsFnPressed()) {
      delta *= ENCODER_FN_MULT;
      keypad.SetKeyPressed();
    }
#endif    
    trx.ChangeFreq(delta);
#ifdef KEYPAD_6
    }
#endif    
  }
#endif    
  UpdateFreq();

  static uint8_t last_tune_in = 0;
  uint8_t new_tune = Modes[trx.state.mode].tx_enable && inTune.Read();
  if (new_tune != last_tune_in && new_tune != trx.Tune)
    trx.Tune = new_tune;
  last_tune_in = new_tune;
  trx.TX = Modes[trx.state.mode].tx_enable && (trx.Tune || inTX.Read());
  outQRP.Write(trx.QRP || trx.Tune);
  OutputTone(PIN_OUT_TONE, trx.Tune);
  outTX.Write(trx.TX);
  UpdateBandCtrl();

  // read and convert smeter
  static long smeter_tm = 0; 
  if (trx.TX) {
    trx.SMeter =  0;
  } else if (millis()-smeter_tm > POLL_SMETER) {
    int val = inSMeter.Read();
    
    bool rev_order = SMeterMap[0] > SMeterMap[14];
    for (int8_t i=14; i >= 0; i--) {
      if ((!rev_order && val > SMeterMap[i]) || (rev_order && val < SMeterMap[i])) {
        trx.SMeter = i+1;
        break;
      }
    }
#ifdef PIN_IN_RIT
    if (trx.RIT)
      trx.RIT_Value = (long)inRIT.ReadRaw()*2*RIT_MAX_VALUE/1024-RIT_MAX_VALUE;
#endif
    smeter_tm = millis();
  }

  // refresh display
#ifndef DISPLAY_DISABLE
  disp.Draw(trx);
#endif  

#ifdef CAT_ENABLE
  // CAT
  if (Serial.available() > 0) {
    ExecCAT();
  }
#endif

  // save state to EEPROM
  if (ee24c32.found()) {
    // save current state to 24C32 (if changed)
    static long state_poll_tm = 0;
    if (millis()-state_poll_tm > POLL_EEPROM_STATE) {
      static uint16_t state_hash = 0;
      static uint8_t state_changed = false;
      static long state_changed_tm = 0;
      uint16_t new_state_hash = trx.StateHash();
      if (new_state_hash != state_hash) {
        state_hash = new_state_hash;
        state_changed = true;
        state_changed_tm = millis();
      } else if (state_changed && (millis()-state_changed_tm > 5000)) {
        // save state
        trx.StateSave(ee24c32);
        state_changed = false;
      }
      state_poll_tm = millis();
    }
  }
  
/*  
  // debug code for measure loop() excecution speed
  // 1.4-7 msec ST7735
  static long zzz=0, last_zzz=0;
  zzz++;
  if (millis()-last_zzz >= 1000) {
    last_zzz=millis();
    trx.state.VFO[trx.GetVFOIndex()^1] = zzz*100;
    zzz=0;
  }
*/  
}
