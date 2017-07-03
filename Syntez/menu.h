///////////////////////////// menu functions ////////////////////////

#include "TinyRTC.h"
#include "utils.h"

char *fmt_hex_val(char *buf, char *title, uint8_t val)
{
  buf = cwr_hex2(cwr_str(buf,title),val);
  *buf++ = 0;
  return buf;
}

void PrintRTCData(RTCData *dt, char *buf, char **items)
{
  *items++ = buf;
  //buf += sprintf(buf,"Day %x ",dt->day)+1;
  *items++ = buf = fmt_hex_val(buf,"Day ",dt->day);
  //buf += sprintf(buf,"Month %x ",dt->month)+1;
  *items++ = buf = fmt_hex_val(buf,"Month ",dt->month);
  //buf += sprintf(buf,"Year %x ",dt->year)+1;
  *items++ = buf = fmt_hex_val(buf,"Year ",dt->year);
  //buf += sprintf(buf,"Hour %x ",dt->hour)+1;
  *items++ = buf = fmt_hex_val(buf,"Hour ",dt->hour);
  //buf += sprintf(buf,"Minute %x ",dt->min)+1;
  *items++ = buf = fmt_hex_val(buf,"Minute ",dt->min);
  //buf += sprintf(buf,"Second %x ",dt->sec)+1;
  fmt_hex_val(buf,"Second ",dt->sec);
  *items = NULL;
}

byte dec2bcd(byte val)
{
  return( (val/10*16) + (val%10));
}

byte bcd2dec(byte val)
{
  return( (val/16*10) + (val%16));
}

void ShowClockMenu()
{
  char title[12];
  strcpy_P(title,PSTR("Clock setup"));
  char help[69];
  strcpy_P(help,PSTR("Lock - exit no save\nAttPre - save & exit\nrotate encoder for change"));
  char buf[64];
  char *items[7];
  RTCData dt;
  byte selected=0;
  long encval;
  RTC_Read(&dt,0,sizeof(dt));
  dt.sec=0;
  disp.clear();
  PrintRTCData(&dt,buf,items);
  disp.DrawMenu(title,(const char**)items,selected,help,2);
  while (1) {
    int keycode=keypad.Read();
    if (keycode >= 0) {
      switch (keycode) {
        case cmdBandUp:
          if (selected > 0) selected--;
          else selected=5;
          disp.DrawMenu(title,(const char**)items,selected,help,2);
          encval=0;
          break;
        case cmdBandDown:
          if (selected < 5) selected++;
          else selected=0;
          disp.DrawMenu(title,(const char**)items,selected,help,2);
          encval=0;
          break;
        case cmdAttPre:
          RTC_Write(&dt);
          return;
        case cmdLock:
          return;
      }
    }
    encval += encoder.GetDelta();
    int delta = encval / 500;
    if (delta != 0) {
      switch (selected) {
        case 0:
          dt.day = dec2bcd(bcd2dec(dt.day)+delta);
          if (dt.day > 0x31) dt.day=0x31;
          break;
        case 1:
          dt.month = dec2bcd(bcd2dec(dt.month)+delta);
          if (dt.month > 0x12) dt.month=0x12;
          break;
        case 2:
          dt.year = dec2bcd(bcd2dec(dt.year)+delta);
          if (dt.year > 0x99) dt.year=0x99;
          break;
        case 3:
          dt.hour = dec2bcd(bcd2dec(dt.hour)+delta);
          if (dt.hour > 0x24) dt.hour=0x24;
          break;
        case 4:
          dt.min = dec2bcd(bcd2dec(dt.min)+delta);
          if (dt.min > 0x59) dt.min=0x59;
          break;
        case 5:
          dt.sec = dec2bcd(bcd2dec(dt.sec)+delta);
          if (dt.sec > 0x59) dt.sec=0x59;
          break;
      }
      PrintRTCData(&dt,buf,items);
      disp.DrawMenu(title,(const char**)items,selected,help,2);
      encval=0;
    }
  }
}

void PrintSMeterData(int *dt, char *buf, char **items)
{
  for (uint8_t i=0; i < 15; i++) {
    *items++ = buf;
    *buf++ = (i < 9 ? 'S' : '+');
    buf = cwr_byte(buf,(i < 9 ? i+1 : i-8));
    if (i >= 9) *buf++ = '0';
    *buf++ = ' ';
    buf = cwr_int(buf,dt[i]);
    *buf++ = 0;
  }
  *items++ = buf;
  cwr_str(buf,"Save & Exit");
  *items = NULL;
}

void ShowSMeterMenu()
{
  char help[70];
  strcpy_P(help,PSTR("BandUp,BandDown - navigation\nAttPre - set value\nLock - exit no save"));
  int smeter[15];
  char buf[145];
  char title[16];
  char *items[17];
  byte selected=0;
  memcpy(smeter,SMeterMap,sizeof(smeter));
  disp.clear();
  PrintSMeterData(smeter,buf,items);
  while (1) {
    //sprintf(title,"AGC=%u   ",inSMeter.Read());
    cwr_str(cwr_int(cwr_str(title,"AGC="),inSMeter.Read()),"   ");
    disp.DrawMenu(title,(const char**)items,selected,help,1);
    int keycode=keypad.Read();
    if (keycode >= 0) {
      switch (keycode) {
        case cmdBandUp:
          if (selected > 0) selected--;
          else selected=15;
          disp.DrawMenu(title,(const char**)items,selected,help,1);
          break;
        case cmdBandDown:
          if (selected < 15) selected++;
          else selected=0;
          disp.DrawMenu(title,(const char**)items,selected,help,1);
          break;
        case cmdAttPre:
          if (selected < 15) {
            smeter[selected]=inSMeter.Read();
            PrintSMeterData(smeter,buf,items);
          } else {
            memcpy(SMeterMap,smeter,sizeof(SMeterMap));
            eeprom_write_block(SMeterMap, SMeterMap_EEMEM, sizeof(SMeterMap));
            return;
          }
          break;
        case cmdLock:
          return;
      }
    }
  }
}

void ShowSi5351CalibrationMenu()
{
  char calibrate_title[17];
  char help[115];
  strcpy_P(calibrate_title,PSTR("CALIBRATE SI5351"));
  strcpy_P(help,PSTR("BandUp - change step\nBandDown - set to zero\nLock - exit no save\nAttPre - save & exit\nrotate encoder for change"));
  // крутим энкодер пока на выходе VFO1 не будет частота "по нулям"
  // потом нажимаем btBandDown
  // выход с отменой - btBandUp
  // brAttPre - смена шага на мелкий/крупный (по дефолту крупный)
  // btVFOSel - сброс в ноль
  long curr_correction = Si5351_correction;
  long last_correction = 0;
  int freq_step = 32;
  disp.clear();
  disp.DrawCalibration(calibrate_title,curr_correction,false,help);
  vfo.set_freq(SI5351_CALIBRATION_FREQ,0,0);
  vfo.set_xtal_freq(SI5351_XTAL_FREQ+curr_correction);
  while (true) {
    curr_correction -= encoder.GetDelta()*freq_step/32;
    disp.DrawCalibration(calibrate_title,curr_correction,freq_step == 1,help);
    if (curr_correction != last_correction) {
      vfo.set_xtal_freq(SI5351_XTAL_FREQ+curr_correction,0);
      last_correction = curr_correction;
    }
    int keycode = keypad.Read();
    if (keycode >= 0) {
      switch (keycode) {
        case cmdBandUp:
          freq_step = (freq_step == 1 ? 32 : 1);
          break;
        case cmdBandDown:
          curr_correction = 0;
          break;
        case cmdLock:
          vfo.set_xtal_freq(SI5351_XTAL_FREQ+Si5351_correction);
          return;
        case cmdAttPre:
          disp.DrawCalibration("SAVE CALIBRATION",curr_correction,false,help);
          Si5351_correction = curr_correction;
          vfo.set_xtal_freq(SI5351_XTAL_FREQ+Si5351_correction);
          eeprom_write_block(&Si5351_correction, &Si5351_correction_EEMEM, sizeof(Si5351_correction));
          delay(2000);
          return;
      }
    }
  }
}

void ShowMenu()
{
  char* MenuItems[] = {"Clock","Si5351","S-Meter","Exit",NULL};
  char title[10];
  char help[54];
  strcpy_P(title,PSTR("Main menu"));
  strcpy_P(help,PSTR("BandUp/BandDown - move\nAttPre - select\nLock - exit"));
  byte selected=0;
  disp.clear();
  disp.DrawMenu(title,MenuItems,selected,help,2);
  while (true) {
    int keycode=keypad.Read();
    if (keycode >= 0) {
      switch (keycode) {
        case cmdBandUp:
          if (selected > 0) selected--;
          else selected=3;
          disp.DrawMenu(title,MenuItems,selected,help,2);
          break;
        case cmdBandDown:
          if (selected < 3) selected++;
          else selected=0;
          disp.DrawMenu(title,MenuItems,selected,help,2);
          break;
        case cmdAttPre:
          switch (selected) {
            case 0:
              if (RTC_found()) ShowClockMenu();
              break;
            case 1:
              ShowSi5351CalibrationMenu();
              break;
            case 2:
              ShowSMeterMenu();
              break;
            case 3:
              return;
          }
          // redraw
          disp.clear();
          disp.DrawMenu(title,MenuItems,selected,help,2);
          break;
        case cmdLock:
          disp.clear();
          return;
      }
    }
  }
}


