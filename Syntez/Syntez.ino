//
// UR5FFR Si570/Si5351 VFO
// v1.1 from 6.08.2017
// Copyright (c) Andrey Belokon, 2016-2017 Odessa 
// https://github.com/andrey-belokon/
// GNU GPL license
//

// !!! all user setting defined in config.h file !!!
#include "config.h"

#include <avr/eeprom.h> 
#include "utils.h"
#include "i2c.h"
#include "pins.h"
#include "Encoder.h"
#ifdef KEYPAD_7
  #include "Keypad_7_I2C.h"
#endif
#ifdef KEYPAD_12
  #include "Keypad_12_I2C.h"
#endif
#include "TinyRTC.h"
#include "Eeprom24C32.h"
#include "TRX.h"
#ifdef DISPLAY_1602
  #include "disp_1602.h"
#endif
#ifdef DISPLAY_ILI9341
  #include "disp_ILI9341.h"
#endif
#ifdef VFO_SI5351
  #include "si5351a.h"
#endif
#ifdef VFO_SI570  
  #include "Si570.h"
#endif

#ifdef KEYPAD_7
  Keypad_7_I2C keypad(I2C_ADR_KEYPAD_7);
#endif
#ifdef KEYPAD_12
  Keypad_12_I2C keypad(I2C_ADR_KEYPAD_12);
#endif

Encoder encoder(ENCODER_PULSE_PER_TURN,ENCODER_FREQ_LO_STEP,ENCODER_FREQ_HI_STEP,ENCODER_FREQ_HI_LO_TRASH);

#ifdef DISPLAY_1602
  Display_1602_I2C disp(I2C_ADR_DISPLAY_1602);
#endif
#ifdef DISPLAY_ILI9341
  Display_ILI9341_SPI disp;
#endif

TRX trx;

Eeprom24C32 ee24c32(I2C_ADR_EE24C32);

#ifdef VFO_SI5351
  Si5351 vfo5351;
#endif
#ifdef VFO_SI570  
  Si570 vfo570;
#endif

int EEMEM SMeterMap_EEMEM[15] = {0};
int SMeterMap[15];

InputPullUpPin inTX(PIN_IN_TX);
InputPullUpPin inTune(PIN_IN_TUNE);
OutputBinPin outTX(PIN_OUT_TX,false,HIGH);
OutputBinPin outQRP(PIN_OUT_QRP,false,HIGH);
InputAnalogPin inRIT(PIN_IN_RIT,5);
InputAnalogPin inSMeter(PIN_IN_SMETER);
OutputTonePin outTone(PIN_OUT_TONE,1000);

OutputPCF8574 outBandCtrl(I2C_ADR_BAND_CTRL,0);

void setup()
{
  i2c_init();
  eeprom_read_block(SMeterMap, SMeterMap_EEMEM, sizeof(SMeterMap));
#ifdef CAT_ENABLE
  Serial.begin(COM_BAUND_RATE);
#endif    
#ifdef VFO_SI5351
  // change for required output level
  vfo5351.setup(
    SI5351_CLK0_DRIVE,
    SI5351_CLK1_DRIVE,
    SI5351_CLK2_DRIVE
  );
  vfo5351.set_xtal_freq(SI5351_CALIBRATION);
#endif  
#ifdef VFO_SI570  
  vfo570.setup(SI570_CALIBRATION);
#endif  
  encoder.setup();
  keypad.setup();
  inTX.setup();
  inTune.setup();
  inRIT.setup();
  outTX.setup();
  outQRP.setup();
  outTone.setup();
  disp.setup();
  if (RTC_found()) {
    ee24c32.setup();
    trx.StateLoad(ee24c32);
  }
}

void vfo_set_freq(long f1, long f2, long f3)
{
#ifdef VFO_SI570  
  vfo570.set_freq(f1);
  #ifdef VFO_SI5351
    vfo5351.set_freq(f2,f3,0);
  #endif
#else
  #ifdef VFO_SI5351
    vfo5351.set_freq(f1,f2,f3);
  #endif
#endif
}

void UpdateFreq() 
{

#ifdef MODE_DC
  vfo_set_freq(
    CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
    0,
    0
  );
#endif

#ifdef MODE_DC_QUADRATURE
  #ifdef VFO_SI5351
    vfo5351.set_freq_quadrature(
      trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0),
      0
    );
  #else
    #error MODE_DC_QUADRATURE allow only with installed Si5351
  #endif
#endif

#ifdef MODE_SINGLE_IF
  #if defined(IFreq_USB) && defined(IFreq_LSB)
    vfo_set_freq( // инверсия боковой - гетеродин сверху
      CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + (trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB) + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
      CLK1_MULT*(trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB),
      0
    );
  #elif defined(IFreq_USB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == LSB) {
      f+=IFreq_USB;
    } else {
      f = abs(IFreq_USB-f);
    }
    vfo_set_freq(CLK0_MULT*f,CLK1_MULT*IFreq_USB,0);
  #elif defined(IFreq_LSB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == USB) {
      f+=IFreq_LSB;
    } else {
      f = abs(IFreq_LSB-f);
    }
    vfo_set_freq(CLK0_MULT*f,CLK1_MULT*IFreq_LSB,0);
  #else
    #error You must define IFreq_LSB/IFreq_USB
  #endif 
#endif

#ifdef MODE_SINGLE_IF_RXTX
  #if defined(IFreq_USB) && defined(IFreq_LSB)
    long f = CLK1_MULT*(trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB);
    vfo_set_freq( // инверсия боковой - гетеродин сверху
      CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + (trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB) + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
      trx.TX ? 0 : f,
      trx.TX ? f : 0
    );
  #elif defined(IFreq_USB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == LSB) {
      f+=IFreq_USB;
    } else {
      f = abs(IFreq_USB-f);
    }
    vfo_set_freq(
      CLK0_MULT*f,
      trx.TX ? 0 : CLK1_MULT*IFreq_USB,
      trx.TX ? CLK1_MULT*IFreq_USB : 0
    );
  #elif defined(IFreq_LSB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == USB) {
      f+=IFreq_LSB;
    } else {
      f = abs(IFreq_LSB-f);
    }
    vfo_set_freq(
      CLK0_MULT*f,
      trx.TX ? 0 : CLK1_MULT*IFreq_LSB,
      trx.TX ? CLK1_MULT*IFreq_LSB : 0
    );
  #else
    #error You must define IFreq_LSB/IFreq_USB
  #endif 
#endif

#ifdef MODE_SINGLE_IF_SWAP
  #if defined(IFreq_USB) && defined(IFreq_LSB)
    long vfo = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + (trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB) + (trx.RIT && !trx.TX ? trx.RIT_Value : 0));
    long f = CLK1_MULT*(trx.state.sideband == LSB ? IFreq_USB : IFreq_LSB);
    vfo_set_freq( // инверсия боковой - гетеродин сверху
      trx.TX ? f : vfo,
      trx.TX ? vfo : f,
      0
    );
  #elif defined(IFreq_USB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == LSB) {
      f+=IFreq_USB;
    } else {
      f = abs(IFreq_USB-f);
    }
    vfo_set_freq(
      trx.TX ? CLK1_MULT*IFreq_USB : CLK0_MULT*f,
      trx.TX ? CLK0_MULT*f : CLK1_MULT*IFreq_USB,
      0
    );
  #elif defined(IFreq_LSB)
    long f = trx.state.VFO[trx.GetVFOIndex()] + (trx.RIT && !trx.TX ? trx.RIT_Value : 0);
    if (trx.state.sideband == USB) {
      f+=IFreq_LSB;
    } else {
      f = abs(IFreq_LSB-f);
    }
    vfo_set_freq(
      trx.TX ? CLK1_MULT*IFreq_LSB : CLK0_MULT*f,
      trx.TX ? CLK0_MULT*f : CLK1_MULT*IFreq_LSB,
      0
    );
  #else
    #error You must define IFreq_LSB/IFreq_USB
  #endif 
#endif

#ifdef MODE_DOUBLE_IF
  long IFreq = (trx.state.sideband == USB ? IFreq_USB : IFreq_LSB);
  vfo_set_freq(
    CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
    CLK1_MULT*(IFreqEx + IFreq),
    CLK2_MULT*(IFreq)
  );
#endif

#ifdef MODE_DOUBLE_IF_LSB
  vfo_set_freq(
    CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
    CLK1_MULT*(IFreqEx + (trx.state.sideband == LSB ? IFreq_LSB : -IFreq_LSB)),
    CLK2_MULT*(IFreq_LSB)
  );
#endif

#ifdef MODE_DOUBLE_IF_USB
  vfo_set_freq(
    CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0)),
    CLK1_MULT*(IFreqEx + (trx.state.sideband == USB ? IFreq_USB : -IFreq_USB)),
    CLK2_MULT*(IFreq_USB)
  );
#endif

#ifdef MODE_DOUBLE_IF_SWAP23
  long IFreq = (trx.state.sideband == USB ? IFreq_USB : IFreq_LSB); 
  long f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0));
  long f2 = CLK1_MULT*(IFreqEx + IFreq);
  long f3 = CLK2_MULT*(IFreq);
  vfo_set_freq(
    f1,
    trx.TX ? f3 : f2,
    trx.TX ? f2 : f3
  );
#endif

#ifdef MODE_DOUBLE_IF_LSB_SWAP23
  long f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0));
  long f2 = CLK1_MULT*(IFreq + (trx.state.sideband == LSB ? IFreq_LSB : -IFreq_LSB));
  long f3 = CLK2_MULT*(IFreq_LSB);
  vfo_set_freq(
    f1,
    trx.TX ? f3 : f2,
    trx.TX ? f2 : f3
  );
#endif

#ifdef MODE_DOUBLE_IF_USB_SWAP23
  long f1 = CLK0_MULT*(trx.state.VFO[trx.GetVFOIndex()] + IFreqEx + (trx.RIT && !trx.TX ? trx.RIT_Value : 0));
  long f2 = CLK1_MULT*(IFreq + (trx.state.sideband == USB ? IFreq_USB : -IFreq_USB));
  long f3 = CLK2_MULT*(IFreq_USB);
  vfo_set_freq(
    f1,
    trx.TX ? f3 : f2,
    trx.TX ? f2 : f3
  );
#endif
}

void UpdateBandCtrl() 
{
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
  outBandCtrl.Set(BCPN_CW, trx.inCW());
  outBandCtrl.Write();
}

#ifdef CAT_ENABLE
  #include "CAT.h"
#endif    

#include "menu.h"

void loop()
{
  bool tune = inTune.Read();
  trx.TX = tune || inTX.Read();
  uint8_t cmd;
  static long last_menu_tm = 0;
  if ((cmd = keypad.Read()) != cmdNone) {
    if (cmd == cmdMenu) {
      // double press at 1sec
      if (millis()-last_menu_tm <= 1000) {
        // call to menu
        ShowMenu();
        // перерисовываем дисплей
        disp.clear();
        disp.reset();
        disp.Draw(trx);
        last_menu_tm = 0;
        return;
      } else {
        last_menu_tm = millis();
      }
    } else {
      trx.ExecCommand(cmd);
    }
#ifdef KEYPAD_12
  } else {
    // при однократном нажатии Menu включаем Lock через 1 сек
    if (last_menu_tm > 0 && millis()-last_menu_tm >= 1000) {
      trx.ExecCommand(cmdLock);
      last_menu_tm = 0;
    }
#endif    
  }
  if (trx.RIT)
    trx.RIT_Value = (long)inRIT.ReadRaw()*2*RIT_MAX_VALUE/1024-RIT_MAX_VALUE;
  long delta = encoder.GetDelta();
  if (delta) {
    if (keypad.IsFnPressed()) {
      delta*=ENCODER_FN_MULT;
      keypad.SetKeyPressed();
    }
    trx.ChangeFreq(delta);
  }
  UpdateFreq();
  outQRP.Write(trx.QRP || tune);
  outTone.Write(tune);
  outTX.Write(trx.TX);
  UpdateBandCtrl();
  // read and convert smeter
  int val = inSMeter.Read();
  trx.SMeter =  0;
  bool rev_order = SMeterMap[0] > SMeterMap[14];
  for (uint8_t i=14; i >= 0; i--) {
    if (!rev_order && val > SMeterMap[i] || rev_order && val < SMeterMap[i]) {
      trx.SMeter =  i+1;
      break;
    }
  }
  // refresh display
  disp.Draw(trx);
#ifdef CAT_ENABLE
  // CAT
  if (Serial.available() > 0) {
    ExecCAT();
  }
#endif
  if (RTC_found()) {
    // save current state to 24C32 (if changed)
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
  }
}
