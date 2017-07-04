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
  strcpy_P(help,PSTR("Up/Down - move\nA/B - save\nLock - exit\nuse encoder for change"));
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
        case cmdVFOSel:
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
  strcpy_P(help,PSTR("Up/Down - move\nLock - exit\nA/B - set value"));
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
        case cmdVFOSel:
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

void ShowSiCalibrationMenu()
{
  char* MenuItems[] = {NULL};
  char title[15];
  char help[22];
  strcpy_P(title,PSTR("Si Calibration"));
  strcpy_P(help,PSTR("press any key to exit"));
  disp.clear();
  byte selected;
  disp.DrawMenu(title,MenuItems,selected,help,2);
#ifdef VFO_SI5351
  vfo5351.out_calibrate_freq();
#endif
#ifdef VFO_SI570
  vfo570.out_calibrate_freq();
#endif
  while (keypad.Read() == cmdNone) ;
}

void ShowMenu()
{
  char* MenuItems[] = {"Clock","S-Meter","Si calibration",NULL};
  char title[10];
  char help[54];
  strcpy_P(title,PSTR("Main menu"));
  strcpy_P(help,PSTR("Up/Down - move\nA/B - select\nLock - exit"));
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
        case cmdVFOSel:
          switch (selected) {
            case 0:
              if (RTC_found()) ShowClockMenu();
              break;
            case 1:
              ShowSMeterMenu();
              break;
            case 2:
              ShowSiCalibrationMenu();
              break;
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


