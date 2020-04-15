///////////////////////////// menu functions ////////////////////////

#ifdef RTC_ENABLE

char *fmt_hex_val(char *buf, const char *title, uint8_t val)
{
  buf = cwr_hex2(cwr_str(buf,title),val);
  *buf++ = 0;
  return buf;
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
  char help[55];
  strcpy_P(help,PSTR("Up/Dn - move\nVFO - save\nMenu - exit\nEncoder - change"));
  char buf[64];
  char *items[6];
  RTCData dt;
  byte selected=0;
  long encval=0;
  char *pb;
  uint8_t keycode;
  int delta;
  RTC_Read(&dt);
  dt.sec=0;
  disp.clear();
  goto l_print;
  while (1) {
    keycode=keypad.Read();
    switch (keycode) {
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=4;
        disp.DrawMenu(title,(const char**)items,selected,help,2);
        encval=0;
        break;
      case cmdBandDown:
        if (selected < 4) selected++;
        else selected=0;
        disp.DrawMenu(title,(const char**)items,selected,help,2);
        encval=0;
        break;
      case cmdVFOSel:
        dt.sec = 0;
        RTC_Write(&dt);
        return;
      case cmdMenu:
      case cmdLock:
        return;
    }
#ifdef ENCODER_ENABLE
    encval += Encoder::GetDelta();
    delta = encval / (ENCODER_FREQ_LO_STEP/6);
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
      }
    l_print:
      pb = buf;
      items[0] = pb;
      //buf += sprintf(buf,"Day %x ",dt->day)+1;
      items[1] = pb = fmt_hex_val(pb,"Day ",dt.day);
      //buf += sprintf(buf,"Month %x ",dt->month)+1;
      items[2] = pb = fmt_hex_val(pb,"Month ",dt.month);
      //buf += sprintf(buf,"Year %x ",dt->year)+1;
      items[3] = pb = fmt_hex_val(pb,"Year ",dt.year);
      //buf += sprintf(buf,"Hour %x ",dt->hour)+1;
      items[4] = pb = fmt_hex_val(pb,"Hour ",dt.hour);
      //buf += sprintf(buf,"Minute %x ",dt->min)+1;
      fmt_hex_val(pb,"Minute ",dt.min);
      items[5] = NULL;
      disp.DrawMenu(title,(const char**)items,selected,help,2);
      encval=0;
#endif    
    }
  }
}

#endif

void ShowSSBDetFreqMenu()
{
  char title[13];
  char help[72];
  char buf[100];
  char *items[6];
  byte mode=0;
  int shift_lsb, shift_usb, shift_rx, shift_rx_init;
  byte selected=0;
  long encval=0;
  int delta;
  char *pb;
  uint32_t freq_lsb, freq_usb;
  strcpy_P(title,PSTR("Tune IF freq"));
  strcpy_P(help,PSTR("Up/Dn - move\nAtt/VFO - change\nMenu - exit\nuse encoder for change"));
  goto l_mode_changed;

  while (1) {
    switch (keypad.Read()) {
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=4;
        goto l_redraw;
      case cmdBandDown:
        if (selected < 4) selected++;
        else selected=0;
        goto l_redraw;
      case cmdRIT:
        switch (selected) {
          case 1:
            shift_lsb = 0;
            break;
          case 2:
            shift_usb = 0;
            break;
          case 3:
            shift_rx = 0;
            break;
        };
        goto l_redraw;
      case cmdVFOSel:
        switch (selected) {
          case 0:
            if (Modes[++mode].name == NULL) mode=0;
            goto l_mode_changed;
          case 1:
            shift_lsb += 20;
            break;
          case 2:
            shift_usb += 20;
            break;
          case 3:
            shift_rx += 20;
            break;
          case 4:
            trx.IFreqShift[mode][0] = shift_lsb;
            trx.IFreqShift[mode][1] = shift_usb;
            trx.IFreqShift[mode][2] = shift_rx;
            trx.IFreqShiftSave(ee24c32);
            break;
        };
        goto l_redraw;
      case cmdAttPre:
        switch (selected) {
          case 0:
            if (mode == 0) {
              // последняя мода
              while (Modes[++mode].name != NULL) ;
            }
            mode--;
            goto l_mode_changed;
          case 1:
            shift_lsb -= 20;
            break;
          case 2:
            shift_usb -= 20;
            break;
          case 3:
            shift_rx -= 20;
            break;
        };
        goto l_redraw;
      case cmdMenu:
      case cmdLock:
        return;
    }
#ifdef ENCODER_ENABLE
    encval += Encoder::GetDelta();
    delta = encval / (ENCODER_FREQ_LO_STEP/100);
    if (delta != 0) {
      switch (selected) {
        case 1:
          shift_lsb += delta;
          break;
        case 2:
          shift_usb += delta;
          break;
        case 3:
          shift_rx += delta;
          break;
      };
      goto l_redraw;
    }
#endif
    continue;
    
    l_mode_changed:
      disp.clear();
      shift_lsb = trx.IFreqShift[mode][0];
      shift_usb = trx.IFreqShift[mode][1];
      shift_rx = trx.IFreqShift[mode][2];
      freq_lsb = Modes[mode].freq[0];
      freq_usb = Modes[mode].freq[1];
      shift_rx_init = Modes[mode].rx_shift;
      
    l_redraw:
      pb=buf;
    
      items[0] = pb;
      pb = cwr_str(pb,"Mode: ");
      pb = cwr_str(pb,Modes[mode].name);
      *pb++ = 0;
      
      items[1] = pb;
      pb = cwr_str(pb,"F LSB: ");
      pb = cwr_long(pb,freq_lsb+shift_lsb);
      *pb++ = 0;
    
      items[2] = pb;
      pb = cwr_str(pb,"F USB: ");
      pb = cwr_long(pb,freq_usb+shift_usb);
      *pb++ = 0;
    
      items[3] = pb;
      pb = cwr_str(pb,"RX Shift: ");
      pb = cwr_long(pb,shift_rx_init+shift_rx);
      *pb++ = 0;
    
      items[4] = pb;
      pb = cwr_str(pb,"Save");
      *pb++ = 0;
      
      items[5] = NULL;

      disp.DrawMenu(title,(const char**)items,selected,help,2);
      encval=0;
  }
}

void PrintSMeterData(int *dt, char *buf, char **items)
{
  for (uint8_t i=0; i < 15; i++) 
    if ((i & 1) == 0) {
      *items++ = buf;
      *buf++ = (i < 9 ? 'S' : '+');
      *buf++ = '0' + (i < 9 ? i+1 : i-8);
      if (i >= 9) *buf++ = '0';
      else *buf++ = ' ';
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
  char help[37];
  strcpy_P(help,PSTR("Up/Dn - move\nMenu - exit\nVFO - set"));
  int smeter[15];
  char buf[100];
  char title[16];
  char *items[10];
  byte selected=0;
  memcpy(smeter,SMeterMap,sizeof(smeter));
  disp.clear();
  PrintSMeterData(smeter,buf,items);
  while (1) {
    //sprintf(title,"AGC=%u   ",inSMeter.Read());
    cwr_str(cwr_int(cwr_str(title,"AGC="),inSMeter.Read()),"   ");
    disp.DrawMenu(title,(const char**)items,selected,help,2);
    uint8_t  keycode=keypad.Read();
    switch (keycode) {
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=8;
        break;
      case cmdBandDown:
        if (selected < 8) selected++;
        else selected=0;
        break;
      case cmdVFOSel:
        if (selected < 8) {
          smeter[selected<<1]=inSMeter.Read();
          PrintSMeterData(smeter,buf,items);
        } else {
          for (byte i=1; i<14; i+=2) 
            smeter[i] = (smeter[i-1]+smeter[i+1]) >> 1;
          memcpy(SMeterMap,smeter,sizeof(SMeterMap));
          eeprom_write_block(SMeterMap, SMeterMap_EEMEM, sizeof(SMeterMap));
          return;
        }
        break;
      case cmdMenu:
      case cmdLock:
        return;
    }
  }
}

void ShowSiCalibrationMenu()
{
  const char* MenuItems[] = {NULL};
  char title[15];
  char help[22];
  strcpy_P(title,PSTR("Si Calibration"));
  strcpy_P(help,PSTR("press any key to exit"));
  disp.clear();
  byte selected=0;
  disp.DrawMenu(title,MenuItems,selected,help,2);
#ifdef VFO_SI5351
  SELECT_SI5351(0);
  vfo5351.out_calibrate_freq();
#endif
#ifdef VFO_SI5351_2
  SELECT_SI5351(1);
  vfo5351_2.out_calibrate_freq();
#endif
#ifdef VFO_SI570
  vfo570.out_calibrate_freq();
#endif
  while (keypad.Read() != cmdNone) ;
  while (keypad.Read() == cmdNone) ;
}

void ShowMenu()
{
  const char* MenuItems[] = {"Clock","S-Meter","Si calibration","IF freq",NULL};
  char title[10];
  char help[40];
  strcpy_P(title,PSTR("Main menu"));
  strcpy_P(help,PSTR("Up/Dn - move\nVFO - select\nMenu - exit"));
  byte selected=0;
  disp.clear();
  while (true) {
    uint8_t  keycode=keypad.Read();
    disp.DrawMenu(title,MenuItems,selected,help,2);
    switch (keycode) {
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=3;
        break;
      case cmdBandDown:
        if (selected < 3) selected++;
        else selected=0;
        break;
      case cmdVFOSel:
        switch (selected) {
          case 0:
#ifdef RTC_ENABLE
            ShowClockMenu();
#endif
            break;
          case 1:
            ShowSMeterMenu();
            break;
          case 2:
            ShowSiCalibrationMenu();
            break;
          case 3:
            ShowSSBDetFreqMenu();
            break;
        }
        // redraw
        disp.clear();
        break;
      case cmdMenu:
      case cmdLock:
        return;
    }
  }
}
