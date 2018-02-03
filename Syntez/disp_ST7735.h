#ifndef DISP_ST7735_H
#define DISP_ST7735_H

// ST7735 SPI TFT. адаптация: Дмитрий Фролов

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "TRX.h"

class Display_ST7735_SPI: public TRXDisplay {
  private:
    uint8_t tx;
  public:
	  void setup();
    void reset();
	  void Draw(TRX& trx);
	  void clear();
	  void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize);
};

//#include "TinyRTC.h"
#include <SPI.h>        // must include this here (or else IDE can't find it)                                         
#include "utils.h"

#define ST7735_CS_PIN    10      // <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define ST7735_DC_PIN    9     // <= DC pin (1=data or 0=command indicator line) also called RS
#define ST7735_RST_PIN   8     // <= RST pin (optional) - not used
#define ST7735_SAVE_SPCR 0     // <= 0/1 with 1 to save/restore AVR SPI control register (to "play nice" when other SPI use)

// ST7735 has several variations, set your version based on this list (using the color of the "tab" on the screen cover).
// NOTE: The tab colors refer to Adafruit versions, other suppliers may vary (you may have to experiment to find the right one).
enum
{
  ST7735_INITB      = 0,        // 1.8" (128x160) ST7735B chipset (only one type)
  ST7735_INITR_GREENTAB   = 1,        // 1.8" (128x160) ST7735R chipset with green tab (same as ST7735_INITR_18GREENTAB)
  ST7735_INITR_REDTAB   = 2,        // 1.8" (128x160) ST7735R chipset with red tab (same as ST7735_INITR_18REDTAB)
  ST7735_INITR_BLACKTAB   = 3,        // 1.8" (128x160) ST7735S chipset with black tab (same as ST7735_INITR_18BLACKTAB)
  ST7735_INITR_144GREENTAB    = 4,        // 1.4" (128x128) ST7735R chipset with green tab
  ST7735_INITR_18GREENTAB   = ST7735_INITR_GREENTAB,  // 1.8" (128x160) ST7735R chipset with green tab
  ST7735_INITR_18REDTAB   = ST7735_INITR_REDTAB,    // 1.8" (128x160) ST7735R chipset with red tab
  ST7735_INITR_18BLACKTAB   = ST7735_INITR_BLACKTAB,  // 1.8" (128x160) ST7735S chipset with black tab
};

#define ST7735_CHIPSET ST7735_INITR_BLACKTAB // <= Set ST7735 LCD chipset/variation here

#include <PDQ_GFX.h>        // PDQ: Core graphics library
#include <PDQ_ST7735.h>      // PDQ: Hardware-specific driver library

#ifdef GFX_FONT_PACKED
#include "font\Tahoma28.pck.h"
#include "font\Tahoma18.pck.h"
#include "font\Tahoma8.pck.h"
#else
#include "font\Tahoma28.h"
#include "font\Tahoma18.h"
#include "font\Tahoma8.h"
#endif

#define color_rgb(r,g,b) (((r)<<11)|((g)<<5)|(b))
#define ST7735_DARKGRAY color_rgb(0x3,0x7,0x3)
#define ST7735_GRAY color_rgb(0x7,0xf,0x7)
#define ST7735_CYAN color_rgb(0x3,0x1F,0x9)
#define ST7735_DARKYELLOW color_rgb(0xf,0x1f,0x0)

PDQ_ST7735 tft;

long cur_freq=0;
long cur_freq2=0;
char cur_freq_buf[9];
int cur_vfo_idx=-1;
byte cur_sideband=0xff;
byte cur_split=0xff;
byte cur_lock=0xff;
byte cur_rit=0xff;
byte cur_tx=0xff;
byte cur_qrp=0xff;
byte cur_attpre=0xFF;
int cur_band=-1;
byte init_smetr=0;
byte cur_sm[15];
int cur_ritval=0xffff;
byte cur_cw=0xff;
long last_tmtm=0;

//bool is_rtc_found;

void Display_ST7735_SPI::setup() 
{
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  //is_rtc_found=RTC_found();
}

void Display_ST7735_SPI::reset()
{
  cur_freq=0;
  cur_freq2=0;
  cur_freq_buf[0]=0;
  cur_vfo_idx=-1;
  cur_sideband=0xff;
  cur_split=0xff;
  cur_lock=0xff;
  cur_rit=0xff;
  cur_tx=0xff;
  cur_qrp=0xff;
  cur_attpre=0xFF;
  cur_band=-1;
  init_smetr=0;
  for (uint8_t i=0; i < 15; i++) cur_sm[i]=0;
  cur_ritval=0xffff;
  cur_cw=0xff;
  last_tmtm=0;
}

void fmt231(char *buf, long v)
{
  buf[8] = 0;
  buf[7] = '0'+v%10; v/=10;
  buf[6] = '.';
  buf[5] = '0'+v%10; v/=10;
  buf[4] = '0'+v%10; v/=10;
  buf[3] = '0'+v%10; v/=10;
  buf[2] = '.';
  buf[1] = '0'+v%10; v/=10;
  if (v > 0) buf[0] = '0'+v;
  else buf[0] = '!';
}

void drawFreq(int x, int y, long f, color_t c)
{
  char bbuf[9];
  char* buf=bbuf;
  char *oldbuf=cur_freq_buf;
  int w=0;
  fmt231(bbuf,f);
  while (*buf && (*buf == *oldbuf)) {
    if (*buf == '.') w+=11;
    else w+=20;
    oldbuf++;
    buf++;
  }
  if (*buf) {
    tft.setTextColor(c);
    tft.setFont(&Tahoma28);
    tft.fillRect(x+w,y-40,160-w,32,ST7735_BLACK);
    tft.setCursor(x+w,y);
    tft.print(buf);
    while (*oldbuf++=*buf++);
  }
}

void drawFreq2(int x, int y, long f2, color_t c)
{
  char buf[9];
  fmt231(buf,f2);
  tft.fillRect(x,y-29,x+13*6+7*2,29,ST7735_BLACK);
  tft.setTextColor(c);
  tft.setFont(&Tahoma18);
  tft.setCursor(x,y);
  tft.print(buf);
}

void drawBtn(int x, int y, uint8_t w, uint8_t h, const char *title, color_t cframe, color_t ctext)
{
  if (cframe == ST7735_BLACK)
  {
    tft.fillRect(x,y,w,h,ST7735_BLACK);
  } else {
    tft.fillRoundRect(x,y,w,h,5,cframe); // +400 byte of code
    //tft.fillRect(x,y,w,h,cframe);
  }
  if (*title) {
    int16_t x1,y1;
    uint16_t w1,h1;
    tft.setTextColor(ctext);
    tft.setFont(&Tahoma8);
    tft.getTextBounds((char *)title,x,y,&x1,&y1,&w1,&h1);
    tft.setCursor(x+((w-w1)/2), y+((h-12)/2)+12);
    tft.print(title);
  }
}

void Display_ST7735_SPI::Draw(TRX& trx) {
  const int FREQ_Y = 0;
  const int FREQ_H = 65; 
  const int BTN_Y = 107;
  const int SMetr_Y = 22;
  const int SMetr_H = 7;
  int vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+50) / 100;
  long f2 = (trx.state.VFO[vfo_idx^1]+50) / 100;

  tft.setTextSize(1);
  
  if (!init_smetr) {
    init_smetr=1;
    tft.setTextColor(ST7735_GREEN);
    tft.setFont(NULL);
    tft.setCursor(7,SMetr_Y);
    tft.print("1");
    tft.setCursor(27,SMetr_Y);
    tft.print("3");
    tft.setCursor(21*2+5,SMetr_Y);
    tft.print("5");
    tft.setCursor(21*3+4,SMetr_Y);
    tft.print("7");
    tft.setCursor(21*4+3,SMetr_Y);
    tft.print("9");
    tft.setTextColor(ST7735_RED);
    tft.setCursor(20*5+1,SMetr_Y);
    tft.print("+20");
    tft.setCursor(20*6+1,SMetr_Y);
    tft.print("+40");
    tft.setCursor(20*7+1,SMetr_Y);
    tft.print("+60");
    for (int j=0; j < 15; j++) 
      tft.drawRect(j*10+5,SMetr_Y+8,9,SMetr_H,(j < 9 ? ST7735_GREEN : ST7735_RED));
    tft.drawFastHLine(0,BTN_Y-3,160,ST7735_GRAY);
    tft.drawFastHLine(0,BTN_Y-2,160,ST7735_GRAY);
  }
 
  for (int j=0; j < 15; j++) {
    if (j < trx.SMeter) {
      if (!cur_sm[j]) {
        tft.fillRect(j*10+5,SMetr_Y+8,9,SMetr_H,(j < 9 ? ST7735_GREEN : ST7735_RED));
        cur_sm[j]=1;
      }
    } else {
      if (cur_sm[j]) {
        tft.fillRect(j*10+6,SMetr_Y+8+1,9-2,SMetr_H-2,ST7735_BLACK);
        cur_sm[j]=0;
      }
    }
  }
 
  if (cur_vfo_idx != vfo_idx) {
    cur_freq=0;
    cur_freq2=0;
    cur_freq_buf[0]=0;
    cur_vfo_idx=vfo_idx;
    tft.fillRect(0,FREQ_Y+38,160,FREQ_H,ST7735_BLACK);
    if (cur_rit) {
      tft.fillRect(0,(cur_vfo_idx ? FREQ_Y+91 : FREQ_Y+56),65,7,ST7735_BLACK);
      cur_ritval=0xffff;
    }
  }
 
  if (f != cur_freq) {
    cur_freq=f;
    if (vfo_idx == 0)
      drawFreq(18,FREQ_Y+78,f,ST7735_YELLOW);
    else
      drawFreq(18,FREQ_Y+106,f,ST7735_YELLOW);
  }
 
  if (f2 != cur_freq2) {
    cur_freq2=f2;
    if (vfo_idx == 0)
      drawFreq2(65,FREQ_Y+103,f2,ST7735_CYAN);
    else
      drawFreq2(65,FREQ_Y+68,f2,ST7735_CYAN);
  }

  if (trx.state.AttPre != cur_attpre) {
    switch (cur_attpre=trx.state.AttPre) {
      case 0:
        drawBtn(0,BTN_Y,30,20,"ATT",ST7735_BLACK,ST7735_DARKGRAY);
        break;
      case 1:
        drawBtn(0,BTN_Y,30,20,"ATT",ST7735_BLUE,ST7735_WHITE);
        break;
      case 2:
        drawBtn(0,BTN_Y,30,20,"PRE",ST7735_GREEN,ST7735_GRAY);
        break;
    }  
  }

  if (trx.QRP != cur_qrp) {
    if (cur_qrp=trx.QRP) 
      drawBtn(43,BTN_Y,30,20,"QRP",ST7735_BLUE,ST7735_WHITE);
    else
      drawBtn(43,BTN_Y,30,20,"QRP",ST7735_BLACK,ST7735_DARKGRAY);
  }
  
  if (trx.RIT != cur_rit) {
    if (cur_rit=trx.RIT) {
      drawBtn(87,BTN_Y,30,20,"RIT",ST7735_BLUE,ST7735_WHITE);
      cur_ritval=0xffff;
    } else {
      drawBtn(87,BTN_Y,30,20,"RIT",ST7735_BLACK,ST7735_DARKGRAY);
      tft.fillRect(0,(cur_vfo_idx ? FREQ_Y+56 : FREQ_Y+91),65,7,ST7735_BLACK);
    }
  }
  
  if (cur_rit && trx.RIT_Value != cur_ritval) {
    int y,v=trx.RIT_Value;
    char buf[16];
    if (cur_vfo_idx == 0) y=FREQ_Y+91;
    else y=FREQ_Y+56;
    cur_ritval=trx.RIT_Value;
    tft.fillRect(0,y,65,7,ST7735_BLACK);
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_CYAN,ST7735_BLACK);
    tft.setCursor(0,y);
    buf[0]='R'; buf[1]='I'; buf[2]='T'; buf[3]=' ';
    if (v == 0) {
      strcpy(buf+4,"0    ");
    } else {
      if (v < 0) {
        buf[4]='-';
        v=-v;
      }
      else buf[4]='+'; 
      //sprintf(buf+5,"%dHz",v);
      //itoa(v,buf+5,10);
      cwr_str(cwr_int(buf+5,v),"Hz");
      //strcat(buf+5,"Hz");
      for (byte i=5; i<=10; i++)
        if (buf[i] == 0) buf[i]=' ';
      buf[11]=0;
    }
    tft.print(buf);
    tft.setTextSize(1);
  }

  if (trx.state.Split != cur_split) {
    if (cur_split=trx.state.Split)
      drawBtn(130,BTN_Y,30,20,"SPL",ST7735_BLUE,ST7735_WHITE);
    else
      drawBtn(130,BTN_Y,30,20,"SPL",ST7735_BLACK,ST7735_DARKGRAY);
  }

  if (trx.Lock != cur_lock) {
    cur_lock=trx.Lock;
    if (cur_lock)
      drawBtn(120,0,40,20,"LOCK",ST7735_RED,ST7735_YELLOW);
    else
      drawBtn(120,0,40,20,"LOCK",ST7735_BLACK,ST7735_DARKGRAY);
  }

  if (trx.TX != cur_tx) {
    if (cur_tx=trx.TX) 
      drawBtn(0,1,30,20,"TX",ST7735_RED,ST7735_YELLOW);
    else
      drawBtn(0,1,30,20,"RX",ST7735_BLACK,ST7735_GREEN);
  }

  uint8_t sb = trx.state.sideband;
  if (trx.BandIndex >= 0 && Bands[trx.BandIndex].sideband != trx.state.sideband) sb |= 0x80;
  if (sb != cur_sideband) {
    char *sb_txt = (trx.state.sideband == LSB ? "LSB" : "USB");
    cur_sideband=sb;
    if (sb & 0x80)
      drawBtn(87,0,30,20,sb_txt,ST7735_RED,ST7735_YELLOW);
    else
      drawBtn(87,0,30,20,sb_txt,ST7735_BLACK,ST7735_BLUE);
  }

 if (trx.BandIndex != cur_band) {
    if ((cur_band=trx.BandIndex) >= 0) {
      int mc = Bands[trx.BandIndex].mc;
      char buf[4];
      buf[3] = 0;
      buf[2] = '0'+mc%10; mc/=10;
      buf[1] = '0'+mc%10; mc/=10;
      if (mc > 0) buf[0] = '0'+mc;
      else buf[0] = '!';
      drawBtn(29,0,30,20,buf,ST7735_BLACK,ST7735_BLUE);
    } else
      drawBtn(29,0,30,20,"",ST7735_BLACK,ST7735_BLUE);
  }

  byte cw=trx.BandIndex >= 0 && Bands[trx.BandIndex].startSSB > 0 &&
          trx.state.VFO[vfo_idx] < Bands[trx.BandIndex].startSSB &&
          trx.state.VFO[vfo_idx] >= Bands[trx.BandIndex].start;
  if (cw != cur_cw) {
    if (cur_cw=cw)
      drawBtn(56,1,30,20,"CW",ST7735_BLACK,ST7735_DARKYELLOW);
    else
      drawBtn(56,1,30,20,"CW",ST7735_BLACK,ST7735_DARKGRAY);
  }
/*
// RTC IS NOT SUPPORTED WHITH ST7735 LCD
  if (is_rtc_found && millis()-last_tmtm > 200) {
    RTCData d;
    char buf[12],*pb;
    last_tmtm=millis();
    RTC_Read(&d,0,sizeof(d));
    //sprintf(buf,"%2x:%02x:%02x",d.hour,d.min,d.sec);
    pb=cwr_hex2sp(buf,d.hour);
    *pb++=':';
    pb=cwr_hex2(pb,d.min);
    *pb++=':';
    pb=cwr_hex2(pb,d.sec);
    *pb=0;
    tft.setFont(NULL);
    tft.setTextSize(2);
    tft.setTextColor(ST7735_CYAN,ST7735_BLACK);
    tft.setCursor(220,13);
    tft.print(buf);
    tft.setTextSize(1);
    tft.setCursor(255,2);
    //sprintf(buf,"%x.%02x.20%02x",d.day,d.month,d.year);
    pb=cwr_hex2sp(buf,d.day);
    *pb++='.';
    pb=cwr_hex2(pb,d.month);
    *pb++='.';
    *pb++='2';
    *pb++='0';
    pb=cwr_hex2(pb,d.year);
    *pb=0;
    tft.print(buf);
  }
*/
}

void Display_ST7735_SPI::clear()
{
  tft.fillScreen(ST7735_BLACK);
}

void Display_ST7735_SPI::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  tft.setFont(NULL);
  tft.setTextSize(1); // override and use always small font size
  tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
  tft.setCursor(0,0);
  if (title) {
    tft.println(title);
    tft.println("");
  }
  for (byte i=0; *items; items++,i++) {
    if (i>7) tft.setCursor(80,(i-4)*8);
    if (i == selected) tft.print(" >");
    else tft.print("  ");
    tft.print(*items);
    tft.println("   ");
  }
  if (help) {
    tft.setCursor(0,96);
    tft.println("");
    tft.println(help);
  }
}

#endif
