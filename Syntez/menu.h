///////////////////////////// menu functions ////////////////////////

char *fmt_hex_val(char *buf, const char *title, uint8_t val)
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
  char help[67];
  strcpy_P(help,PSTR("Up/Down - move\nA/B - save\nFn/Lock - exit\nuse encoder for change"));
  char buf[64];
  char *items[7];
  RTCData dt;
  byte selected=0;
  long encval=0;
  RTC_Read(&dt);
  dt.sec=0;
  PrintRTCData(&dt,buf,items);
  disp.clear();
  disp.DrawMenu(title,(const char**)items,selected,help,2);
  while (1) {
    uint8_t keycode=keypad.Read();
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
      case cmdMenu:
      case cmdLock:
        return;
    }
#ifdef ENCODER_ENABLE
    encval += encoder.GetDelta();
    int delta = encval / (ENCODER_FREQ_LO_STEP/6);
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
#endif    
    }
  }
}

void PrintSSBFreqData(
#ifdef SSBDetectorFreq_LSB
  int LSB_freq, 
#endif
#ifdef SSBDetectorFreq_USB
  int USB_freq, 
#endif
  char *buf, char **items)
{
  char *pb=buf;
#ifdef SSBDetectorFreq_LSB  
  *items++ = pb;
  pb = cwr_str(pb,"LSB:  ");
  pb = cwr_long(pb,SSBDetectorFreq_LSB+LSB_freq);
  *pb++ = ' ';
  *pb++ = ' ';
  *pb++ = 0;
#endif  
#ifdef SSBDetectorFreq_USB  
  *items++ = pb;
  pb = cwr_str(pb,"USB:  ");
  pb = cwr_long(pb,SSBDetectorFreq_USB+USB_freq);
  *pb++ = ' ';
  *pb++ = ' ';
  *pb++ = 0;
#endif  
  *items++ = NULL;
}

void ShowSSBDetFreqMenu()
{
  char title[13];
  char help[84];
  char buf[40];
  char *items[3];
  int LSB_shift=SSBShift_LSB, USB_shift=SSBShift_USB;
  byte selected=0;
  long encval=0;
  int delta;
  strcpy_P(title,PSTR("Tune IF freq"));
#if defined(SSBDetectorFreq_LSB) && defined(SSBDetectorFreq_USB)
  strcpy_P(help,PSTR("Up/Down - move\nA=B - reset\nA/B - save & exit\nFn/Lock - exit no save\nuse encoder for change"));
#else  
  strcpy_P(help,PSTR("A=B - reset\nFn/Lock - exit no save\nA/B - save & exit\nuse encoder for change"));
#endif    
  disp.clear();
  goto l_redraw_full;

  while (1) {
    switch (keypad.Read()) {
#if defined(SSBDetectorFreq_LSB) && defined(SSBDetectorFreq_USB)
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=1;
        goto l_redraw;
      case cmdBandDown:
        if (selected < 1) selected++;
        else selected=0;
        goto l_redraw;
#endif        
      case cmdVFOEQ:
        LSB_shift=USB_shift=0;
        goto l_redraw_full;
      case cmdVFOSel:
        SSBShift_LSB=LSB_shift;
        SSBShift_USB=USB_shift;
        eeprom_write_word((uint16_t*)&SSBShift_LSB_EEMEM, (uint16_t)SSBShift_LSB);
        eeprom_write_word((uint16_t*)&SSBShift_USB_EEMEM, (uint16_t)SSBShift_USB);
        return;
      case cmdMenu:
      case cmdLock:
        return;
    }
#ifdef ENCODER_ENABLE
    encval += encoder.GetDelta();
    delta = encval / (ENCODER_FREQ_LO_STEP/100);
    if (delta != 0) {
#if defined(SSBDetectorFreq_LSB) && defined(SSBDetectorFreq_USB)
      switch (selected) {
        case 0:
          LSB_shift+=delta;
          break;
        case 1:
          USB_shift+=delta;
          break;
      }
#elif defined(SSBDetectorFreq_LSB)
      LSB_shift+=delta;
#elif defined(SSBDetectorFreq_USB)
      USB_shift+=delta;
#endif
#endif    
    l_redraw_full:
      PrintSSBFreqData(
#ifdef SSBDetectorFreq_LSB
        LSB_shift, 
#endif    
#ifdef SSBDetectorFreq_USB
        USB_shift, 
#endif    
        buf, items);
    l_redraw:
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
  char help[48];
  strcpy_P(help,PSTR("Up/Down - move\nFn/Lock - exit\nA/B - set value"));
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
    uint8_t  keycode=keypad.Read();
    switch (keycode) {
      case cmdBandUp:
        if (selected > 0) selected--;
        else selected=15;
        break;
      case cmdBandDown:
        if (selected < 15) selected++;
        else selected=0;
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
  vfo5351.out_calibrate_freq();
#endif
#ifdef VFO_SI570
  vfo570.out_calibrate_freq();
#endif
  while (keypad.Read() == cmdNone) ;
}

void ShowMenu()
{
  const char* MenuItems[] = {"Clock","S-Meter","Si calibration","SSB freq",NULL};
  char title[10];
  char help[45];
  strcpy_P(title,PSTR("Main menu"));
  strcpy_P(help,PSTR("Up/Down - move\nA/B - select\nFn/Lock - exit"));
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
        disp.clear();
        return;
    }
  }
}
