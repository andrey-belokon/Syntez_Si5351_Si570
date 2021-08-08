#include "disp_oled128x64.h"

#if defined(DISPLAY_OLED128x64) || defined(DISPLAY_OLED_SH1106_128x64)

// SSD1306Ascii library https://github.com/greiman/SSD1306Ascii
#include <SSD1306AsciiAvrI2c.h>
#include "fonts\lcdnums14x24mod.h"
#include "fonts\quad7x8.h"

#ifdef RTC_ENABLE
  #include "RTC.h"
#endif
#include "utils.h"

long cur64_freq=0;
long cur64_freq2=0;
char cur64_freq_buf[8];
byte cur64_sideband=0xff;
byte cur64_split=0xff;
byte cur64_lock=0xff;
byte cur64_rit=0xff;
byte cur64_tx=0xff;
byte cur64_qrp=0xff;
byte cur64_attpre=0xFF;
byte init_smetr64=0;
byte cur64_gc=0xFF;
byte cur64_sm[15];
int cur64_ritval=0xffff;
byte cur64_mode=0xff;
long last_tmtm64=0;
byte last_brightness64=0;

SSD1306AsciiAvrI2c oled64;

void Display_OLED128x64::setBright(uint8_t brightness)
{
  if (brightness < 0) brightness = 0;
  if (brightness > 15) brightness = 15;
  if (brightness == 0) 
    oled64.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
  else {
    if (last_brightness64 == 0)
      oled64.ssd1306WriteCmd(SSD1306_DISPLAYON);
    oled64.setContrast(brightness << 4);
  }
  last_brightness64 = brightness; 
}

void Display_OLED128x64::setup() 
{
#ifdef DISPLAY_OLED128x64
  oled64.begin(&Adafruit128x64, I2C_ADD_DISPLAY_OLED);
#endif
#ifdef DISPLAY_OLED_SH1106_128x64
  oled64.begin(&SH1106_128x64, I2C_ADD_DISPLAY_OLED);
#endif
  reset();
  clear();
  last_brightness64=0;
}

void Display_OLED128x64::reset() 
{
  cur64_freq=0;
  cur64_freq2=0;
  cur64_freq_buf[0]=0;
  cur64_sideband=0xff;
  cur64_split=0xff;
  cur64_lock=0xff;
  cur64_rit=0xff;
  cur64_tx=0xff;
  cur64_qrp=0xff;
  cur64_gc=0xFF;
  cur64_attpre=0xFF;
  init_smetr64=0;
  for (uint8_t i=0; i < 15; i++) cur64_sm[i]=0;
  cur64_ritval=0xffff;
  cur64_mode=0xff;
  last_tmtm64=0;
}

void Display_OLED128x64::Draw(TRX& trx) 
{
  int vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+5) / 10;
  
  if (f != cur64_freq) {
    char buf[10];
    buf[8] = '0'+f%10; f/=10;
    buf[7] = '0'+f%10; f/=10;
    buf[6] = '.';
    buf[5] = '0'+f%10; f/=10;
    buf[4] = '0'+f%10; f/=10;
    buf[3] = '0'+f%10; f/=10;
    buf[2] = '.';
    buf[1] = '0'+f%10; f/=10;
    if (f > 0) buf[0] = '0'+f;
    else buf[0] = ':';
    buf[9] = 0;
    oled64.setFont(lcdnums14x24mod);
    oled64.setCursor(1, 2);
    oled64.print(buf);
    cur64_freq = f;
  }

  oled64.setFont(X11fixed7x14);
  
  if (trx.TX != cur64_tx) {
    oled64.setCursor(0,0);
    if ((cur64_tx=trx.TX) != 0) oled64.print("TX");
    else oled64.print("RX");
  }

  if (trx.Lock != cur64_lock) {
    oled64.setCursor(25,0);
    char bb[2];
    bb[0]=0x24; bb[1]=0;
    if ((cur64_lock=trx.Lock) != 0) oled64.print(bb); // dollar
    else oled64.print(" ");
  }
  
  uint8_t mode = trx.state.mode;
  if (mode != cur64_mode) {
    cur64_mode = mode;
    oled64.setCursor(39,0);
    const char *p = Modes[trx.state.mode].name;
    char bb[4];
    bb[1]=bb[2]=' '; bb[3]=0;
    bb[0] = *p++;
    if (*p) {
      bb[1] = *p++;  
      if (*p) {
        bb[2] = *p;
      }
    }
    oled64.print(bb);
  }

  uint8_t new_gc = trx.BandIndex < 0;
  if (new_gc != cur64_gc) {
    oled64.setCursor(68,0);
    if ((cur64_gc = new_gc) != 0) oled64.print("GC");
    else oled64.print("  ");
  }

#ifdef RTC_ENABLE
  if (millis()-last_tmtm64 > 200) {
    RTCData d;
    char buf[7],*pb;
    last_tmtm64=millis();
    RTC_Read(&d);   
    //sprintf(buf,"%2x:%02x",d.hour,d.min);
    pb=cwr_hex2sp(buf,d.hour);
    if (millis()/1000 & 1) *pb++=':';
    else *pb++=' ';
    pb=cwr_hex2(pb,d.min);
    oled64.setCursor(128-35,0);
    oled64.print(buf);
  }
#endif

  oled64.setFont(quad7x8);

  if (!init_smetr64) {
    init_smetr64=1;
    for (byte j=0; j < 15; j++) {
      byte x = 4+j*8;
      if (j < 9 || !(j&1)) {
        oled64.setCursor(x,5);
        oled64.write(0x23);
        oled64.setCursor(x,6);
        oled64.write(0x24);
      }
    }
  }
  
  for (byte j=0; j < 15; j++) {
    byte x = 4+j*8;
    if (j < trx.SMeter) {
      if (!cur64_sm[j]) {
        oled64.setCursor(x,5);
        oled64.write(0x21);
        oled64.setCursor(x,6);
        oled64.write(0x22);
        cur64_sm[j]=1;
      }
    } else {
      if (cur64_sm[j]) {
        if (j < 9 || !(j&1)) {
          oled64.setCursor(x,5);
          oled64.write(0x23);
          oled64.setCursor(x,6);
          oled64.write(0x24);
        } else {
          oled64.setCursor(x,5);
          oled64.write(0x20);
          oled64.setCursor(x,6);
          oled64.write(0x20);
        }
        cur64_sm[j]=0;
      }
    }
  }
  
  oled64.setFont(font5x7);

  if (trx.state.AttPre != cur64_attpre) {
    oled64.setCursor(4,7);
    switch (cur64_attpre=trx.state.AttPre) {
      case 0:
        oled64.print("   ");
        break;
      #ifdef MODE_SUPER21
        case 1:
        case 2:
          oled64.print("AT");
          oled64.print(char('0'+cur64_attpre));
          break;
        case 3:
          oled64.print("PRE");
          break;
      #else
        case 1:
          oled64.print("ATT");
          break;
        case 2:
          oled64.print("PRE");
          break;
      #endif
    }  
  }

  if ((trx.QRP | trx.Tune) != cur64_qrp) {
    oled64.setCursor(28+4*2,7);
    if ((cur64_qrp = trx.QRP | trx.Tune) != 0) oled64.print("QRP");
    else oled64.print("   ");
  }
  
  if (trx.RIT != cur64_rit) {
    oled64.setCursor(56+4*3,7);
    if ((cur64_rit=trx.RIT) != 0) oled64.print("RIT");
    else oled64.print("   ");
  }

  if (trx.state.Split != cur64_split) {
    oled64.setCursor(84+4*4,7);
    if ((cur64_split=trx.state.Split) != 0) oled64.print("SPL");
    else oled64.print("   ");
  }
}

void Display_OLED128x64::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  oled64.setFont(font5x7);
  (void)(fontsize); // supress warning about unused params
  
  oled64.setCursor(0,0);
  if (title) {
    oled64.println(title);
    //oled64.println("");
  }
  
  int help_lines = 1; // \n между строками +1
  for (const char *p=help; *p; p++)
    if (*p == '\n') help_lines++;
  int item_lines = 8-help_lines-1;
  int item_cnt = 0;
  for (byte i=0; items[i]; i++) item_cnt++;
  int start_i = 0;
  if (item_cnt > item_lines) {
    start_i = selected-item_lines+1;
    if (start_i < 0) 
      start_i = 0;
  }
  
  for (byte i=0; i < item_lines; i++, start_i++) {
    if (start_i == selected) oled64.print(" >");
    else oled64.print("  ");
    oled64.print(items[start_i]);
    oled64.println("   ");
  }
  
  if (help) {
    //oled64.println("");
    oled64.setCursor(0,8-help_lines);
    oled64.println(help);
  }
}

void Display_OLED128x64::clear()
{
  oled64.clear();
}

#endif
