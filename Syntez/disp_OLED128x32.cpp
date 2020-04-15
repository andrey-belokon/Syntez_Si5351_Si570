#include "disp_oled128x32.h"
// SSD1306Ascii library https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiAvrI2c.h"
#include "fonts\lcdnums12x16mod.h"
#include "fonts\quad7x8.h"

#ifdef RTC_ENABLE
  #include "RTC.h"
#endif
#include "utils.h"

SSD1306AsciiAvrI2c oled32;

long cur32_freq=0;
long cur32_freq2=0;
char cur32_freq_buf[8];
byte cur32_sideband=0xff;
byte cur32_split=0xff;
byte cur32_lock=0xff;
byte cur32_rit=0xff;
byte cur32_tx=0xff;
byte cur32_qrp=0xff;
byte cur32_attpre=0xFF;
byte init_smetr32=0;
byte cur32_gc=0xFF;
byte cur32_sm[15];
int cur32_ritval=0xffff;
byte cur32_mode=0xff;
long last_tmtm32=0;
byte last_brightness32=0;

void Display_OLED128x32::setBright(uint8_t brightness)
{
  if (brightness < 0) brightness = 0;
  if (brightness > 15) brightness = 15;
  if (brightness == 0) 
    oled32.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
  else {
    if (last_brightness32 == 0)
      oled32.ssd1306WriteCmd(SSD1306_DISPLAYON);
    oled32.setContrast(brightness << 4);
  }
  last_brightness32 = brightness; 
}

void Display_OLED128x32::setup() 
{
  oled32.begin(&Adafruit128x32, I2C_ADD_DISPLAY_OLED);
  reset();
  clear();
  last_brightness32=0;
}

void Display_OLED128x32::reset() 
{
  cur32_freq=0;
  cur32_freq2=0;
  cur32_freq_buf[0]=0;
  cur32_sideband=0xff;
  cur32_split=0xff;
  cur32_lock=0xff;
  cur32_rit=0xff;
  cur32_tx=0xff;
  cur32_qrp=0xff;
  cur32_gc=0xFF;
  cur32_attpre=0xFF;
  init_smetr32=0;
  for (uint8_t i=0; i < 15; i++) cur32_sm[i]=0;
  cur32_ritval=0xffff;
  cur32_mode=0xff;
  last_tmtm32=0;
}

void Display_OLED128x32::Draw(TRX& trx) 
{
  int vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+5) / 10;
  
  if (f != cur32_freq) {
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
    oled32.setFont(lcdnums12x16mod);
    oled32.setCursor(20, 0);
    oled32.print(buf);
    cur32_freq = f;
  }

  oled32.setFont(quad7x8);

  if (!init_smetr32) {
    init_smetr32=1;
    for (byte j=0; j < 15; j++) {
      byte x = 4+j*8;
      if (j < 9 || !(j&1)) {
        oled32.setCursor(x,2);
        oled32.write(0x26);
      }
    }
  }
  
  for (byte j=0; j < 15; j++) {
    byte x = 4+j*8;
    if (j < trx.SMeter) {
      if (!cur32_sm[j]) {
        oled32.setCursor(x,2);
        oled32.write(0x25);
        cur32_sm[j]=1;
      }
    } else {
      if (cur32_sm[j]) {
        if (j < 9 || !(j&1)) {
          oled32.setCursor(x,2);
          oled32.write(0x26);
        } else {
          oled32.setCursor(x,2);
          oled32.write(0x20);
        }
        cur32_sm[j]=0;
      }
    }
  }
  
  oled32.setFont(font5x7);
 
  if (trx.TX != cur32_tx) {
    oled32.setCursor(0,0);
    if ((cur32_tx=trx.TX) != 0) oled32.print("TX");
    else oled32.print("RX");
  }

  if (trx.Lock != cur32_lock) {
    oled32.setCursor(13,0);
    char bb[2];
    bb[0]=0x24; bb[1]=0;
    if ((cur32_lock=trx.Lock) != 0) oled32.print(bb); // dollar
    else oled32.print(" ");
  }
  
  if (trx.state.AttPre != cur32_attpre) {
    oled32.setCursor(0,1);
    switch (cur32_attpre=trx.state.AttPre) {
      case 0:
        oled32.print("   ");
        break;
      case 1:
        oled32.print("ATT");
        break;
      case 2:
        oled32.print("PRE");
        break;
    }  
  }

  uint8_t new_gc = trx.BandIndex < 0;
  if (new_gc != cur32_gc) {
    oled32.setCursor(0,3);
    if ((cur32_gc = new_gc) != 0) oled32.print("G");
    else oled32.print(" ");
  }

  uint8_t mode = trx.state.mode;
  if (mode != cur32_mode) {
    cur32_mode = mode;
    oled32.setCursor(9,3);
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
    oled32.print(bb);
  }

  if ((trx.QRP | trx.Tune) != cur32_qrp) {
    oled32.setCursor(33,3);
    if ((cur32_qrp = trx.QRP | trx.Tune) != 0) oled32.print("QRP");
    else oled32.print("   ");
  }
  
  if (trx.RIT != cur32_rit) {
    oled32.setCursor(55,3);
    if ((cur32_rit=trx.RIT) != 0) oled32.print("RIT");
    else oled32.print("   ");
  }

  if (trx.state.Split != cur32_split) {
    oled32.setCursor(77,3);
    if ((cur32_split=trx.state.Split) != 0) oled32.print("SPL");
    else oled32.print("   ");
  }
  
#ifdef RTC_ENABLE
  if (millis()-last_tmtm32 > 200) {
    RTCData d;
    char buf[7],*pb;
    last_tmtm32=millis();
    RTC_Read(&d);   
    //sprintf(buf,"%2x:%02x",d.hour,d.min);
    pb=cwr_hex2sp(buf,d.hour);
    if (millis()/1000 & 1) *pb++=':';
    else *pb++=' ';
    pb=cwr_hex2(pb,d.min);
    oled32.setCursor(128-25-4,3);
    oled32.print(buf);
  }
#endif
}

void Display_OLED128x32::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  oled32.setFont(font5x7);
  (void)(fontsize); // supress warning about unused params
  
  oled32.setCursor(0,0);
  if (title) {
    oled32.println(title);
    //oled32.println("");
  }
  
  int item_lines = 3;
  int item_cnt = 0;
  for (byte i=0; items[i]; i++) item_cnt++;
  int start_i = 0;
  if (item_cnt > item_lines) {
    start_i = selected-item_lines+1;
    if (start_i < 0) 
      start_i = 0;
  }
  
  for (byte i=0; i < item_lines; i++, start_i++) {
    if (start_i == selected) oled32.print(" >");
    else oled32.print("  ");
    oled32.print(items[start_i]);
    oled32.println("   ");
  }
}

void Display_OLED128x32::clear()
{
  oled32.clear();
}
