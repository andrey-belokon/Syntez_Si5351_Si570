//
// UR5FFR Si570/Si5351 VFO
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
#include "Keypad_I2C.h"
#include "TinyRTC.h"
#include "Eeprom24C32.h"
#include "TRX.h"
#include "disp_ILI9341.h"
#ifdef VFO_SI5351
  #include "si5351a.h"
#endif
#ifdef VFO_SI570  
  #include "Si570.h"
#endif

/*
  I2C address mapping
  I2C device found at address 0x3B  ! PCF8574
  I2C device found at address 0x3E  ! PCF8574
  I2C device found at address 0x50  ! 24C04 at TinyRTC board [optional]
  I2C device found at address 0x55  ! Si570 [optional]
  I2C device found at address 0x60  ! Si5351 [optional]
  I2C device found at address 0x68  ! DS1307 at TinyRTC board [optional]
*/

KeypadI2C keypad(0x3E);
Encoder encoder(ENCODER_PULSE_PER_TURN);
Display_ILI9341_SPI disp;
TRX trx;
Eeprom24C32 ee24c32(0x50);

#ifdef VFO_SI5351
  Si5351 vfo5351;
#endif
#ifdef VFO_SI570  
  Si570 vfo570;
#endif

int EEMEM SMeterMap_EEMEM[15] = {0};
int SMeterMap[15];

InputPullUpPin inTX(4);
InputPullUpPin inTune(5);
OutputBinPin outTX(6,false,HIGH);
OutputBinPin outQRP(7,false,HIGH);
InputAnalogPin inRIT(A7,5);
InputAnalogPin inSMeter(A6);
OutputTonePin outTone(8,1000);

OutputPCF8574 outBandCtrl(0x3B,0);
// распиновка I2C расширителя:
// двоичный дешифратор диапазона - пины 0-3
const int pnBand0 = 0;
const int pnBand1 = 1;
const int pnBand2 = 2;
const int pnBand3 = 3;
// 5й пин - ATT, 6й пин - Preamp
const int pnAtt = 5;
const int pnPre = 6;

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
  // 0-nothing; 1-ATT; 2-Preamp
  switch (trx.state.AttPre) {
    case 0:
      outBandCtrl.Set(pnAtt,false);
      outBandCtrl.Set(pnPre,false);
      break;
    case 1:
      outBandCtrl.Set(pnAtt,true);
      outBandCtrl.Set(pnPre,false);
      break;
    case 2:
      outBandCtrl.Set(pnAtt,false);
      outBandCtrl.Set(pnPre,true);
      break;
  }
  outBandCtrl.Set(pnBand0, trx.BandIndex & 0x1);
  outBandCtrl.Set(pnBand1, trx.BandIndex & 0x2);
  outBandCtrl.Set(pnBand2, trx.BandIndex & 0x4);
  outBandCtrl.Set(pnBand3, trx.BandIndex & 0x8);
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
  if ((cmd = keypad.Read()) != cmdNone) {
    if (cmd == cmdMenu) {
      static long last_menu_tm = 0;
      // double press at 1sec
      if (millis()-last_menu_tm <= 1000) {
        // call to menu
        ShowMenu();
        // перерисовываем дисплей
        disp.clear();
        disp.reset();
        disp.Draw(trx);
        return;
      } else {
        last_menu_tm = millis();
      }
    } else {
      trx.ExecCommand(cmd);
    }
  }
  if (trx.RIT)
    trx.RIT_Value = (long)inRIT.ReadRaw()*2*RIT_MAX_VALUE/1024-RIT_MAX_VALUE;
  long delta = encoder.GetDelta();
  if (delta) {
    if (keypad.IsFnPressed()) {
      delta*=10;
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
  if (Serial.available() > 0)
    ExecCAT();
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
