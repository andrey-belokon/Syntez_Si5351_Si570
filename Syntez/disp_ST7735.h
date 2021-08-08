#ifndef DISP_ST7735_H
#define DISP_ST7735_H

// ST7735 SPI TFT. начальная адаптация: Дмитрий Фролов

#include <Arduino.h>
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

#ifdef RTC_ENABLE
  #include "RTC.h"
#endif
#include <SPI.h>        // must include this here (or else IDE can't find it)                                         
#include "utils.h"

#define ST7735_CS_PIN    10      // <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define ST7735_DC_PIN    9     // <= DC pin (1=data or 0=command indicator line) also called RS
//#define ST7735_RST_PIN   8     // <= RST pin (optional) - not used
#define ST7735_SAVE_SPCR 0     // <= 0/1 with 1 to save/restore AVR SPI control register (to "play nice" when other SPI use)

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
char cur_freq_buf[8];
byte cur_sideband=0xff;
byte cur_split=0xff;
byte cur_lock=0xff;
byte cur_rit=0xff;
byte cur_tx=0xff;
byte cur_qrp=0xff;
byte cur_attpre=0xFF;
byte init_smetr=0;
byte cur_gc=0xFF;
byte cur_sm[15];
int cur_ritval=0xffff;
byte cur_mode=0xff;
long last_tmtm=0;

void Display_ST7735_SPI::setup() 
{
  tft.begin();
#ifdef TFT_ORIENTATION
  tft.setRotation((TFT_ORIENTATION << 1)+1);
#else
  tft.setRotation(3);
#endif  
  tft.fillScreen(ST7735_BLACK);
}

void Display_ST7735_SPI::reset()
{
  cur_freq=0;
  cur_freq2=0;
  cur_freq_buf[0]=0;
  cur_sideband=0xff;
  cur_split=0xff;
  cur_lock=0xff;
  cur_rit=0xff;
  cur_tx=0xff;
  cur_qrp=0xff;
  cur_gc=0xFF;
  cur_attpre=0xFF;
  init_smetr=0;
  for (uint8_t i=0; i < 15; i++) cur_sm[i]=0;
  cur_ritval=0xffff;
  cur_mode=0xff;
  last_tmtm=0;
}

void fmt232(char *buf, long v)
{
  buf[9] = 0;
  buf[8] = '0'+v%10; v/=10;
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
  char bbuf[10];
  char* buf=bbuf;
  char *oldbuf=cur_freq_buf;
  int w=0;
  fmt232(bbuf,f);
  bbuf[2]=' '; bbuf[6]=0;
  while (*buf && (*buf == *oldbuf)) {
    if (*buf == ' ') w+=8;
    else w+=20;
    oldbuf++;
    buf++;
  }
  tft.setTextColor(c);
  if (*buf) {
    tft.setFont(&Tahoma28);
    tft.fillRect(x+w,y-40,160-w,32,ST7735_BLACK);
    tft.setCursor(x+w,y);
    tft.print(buf);
    while ((*oldbuf++=*buf++) != 0);
  }
  tft.setFont(&Tahoma18);
  tft.fillRect(x+113,y-40,160-116,25,ST7735_BLACK);
  tft.setCursor(x+113,y-13);
  tft.print(bbuf+7);
}

void drawFreq2(int x, int y, long f2, color_t c)
{
  char buf[10];
  fmt232(buf,f2);
  buf[8]=0;
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

void drawBtn3020(int x, int y,  const char *title, uint8_t pressed)
{
  if (pressed)
      drawBtn(x,y,30,20,title,ST7735_BLUE,ST7735_WHITE);
    else
      drawBtn(x,y,30,20,title,ST7735_BLACK,ST7735_DARKGRAY);
}

void Display_ST7735_SPI::Draw(TRX& trx) {
  const int FREQ_Y = 0;
  const int BTN_Y = 107;
  const int SMetr_Y = 22;
  const int SMetr_H = 7;
  int vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+5) / 10;
  long f2 = (trx.state.VFO[vfo_idx^1]+5) / 10;

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
 
  if (f != cur_freq) {
    cur_freq=f;
    drawFreq(18,FREQ_Y+78,f,ST7735_YELLOW);
  }
 
  if (f2 != cur_freq2) {
    cur_freq2=f2;
    drawFreq2(65,FREQ_Y+103,f2,ST7735_CYAN);
  }

  if (trx.state.AttPre != cur_attpre) {
    switch (cur_attpre=trx.state.AttPre) {
      case 0:
        drawBtn(0,BTN_Y,30,20,"ATT",ST7735_BLACK,ST7735_DARKGRAY);
        break;
      #ifdef MODE_SUPER21
        case 1:
        case 2:
          char buf[4];
          buf[0]='A';
          buf[1]='T';
          buf[2]='0'+cur_attpre;
          buf[3]=0;
          drawBtn(0,BTN_Y,30,20,buf,ST7735_BLUE,ST7735_WHITE);
          break;
        case 3:
          drawBtn(0,BTN_Y,30,20,"PRE",ST7735_GREEN,ST7735_GRAY);
          break;
      #else
        case 1:
          drawBtn(0,BTN_Y,30,20,"ATT",ST7735_BLUE,ST7735_WHITE);
          break;
        case 2:
          drawBtn(0,BTN_Y,30,20,"PRE",ST7735_GREEN,ST7735_GRAY);
          break;
      #endif
    }  
  }

  if ((trx.QRP | trx.Tune) != cur_qrp) {
    cur_qrp = trx.QRP | trx.Tune;
    drawBtn3020(32,BTN_Y,"QRP",cur_qrp);
  }
  
  if (trx.RIT != cur_rit) {
    cur_rit=trx.RIT;
    drawBtn3020(65,BTN_Y,"RIT",cur_rit);
    if (cur_rit) {
      cur_ritval=0xffff;
    } else {
      tft.fillRect(0,FREQ_Y+91,65,7,ST7735_BLACK);
    }
  }
  
  if (cur_rit && trx.RIT_Value != cur_ritval) {
    int v=trx.RIT_Value;
    char buf[14];
    cur_ritval=trx.RIT_Value;
    if (v == 0) {
      cwr_str(buf,"0    ");
    } else {
      if (v < 0) {
        buf[0]='-';
        v=-v;
      }
      else buf[0]='+'; 
      cwr_str(cwr_int(buf+1,v),"Hz  ");
    }
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_BLUE,ST7735_BLACK);
    tft.setCursor(10,FREQ_Y+91);
    tft.print(buf);
    tft.setTextSize(1);
  }

  if (trx.state.Split != cur_split) {
    cur_split=trx.state.Split;
    drawBtn3020(98,BTN_Y,"SPL",cur_split);
  }

  if (trx.Lock != cur_lock) {
    cur_lock=trx.Lock;
    drawBtn3020(130,BTN_Y,"LCK",cur_lock);
  }

  if (trx.TX != cur_tx) {
    if ((cur_tx=trx.TX) != 0) 
      drawBtn(0,0,30,16,"TX",ST7735_RED,ST7735_YELLOW);
    else
      drawBtn(0,0,30,16,"RX",ST7735_BLACK,ST7735_GREEN);
  }

  uint8_t mode = trx.state.mode;
  if (mode != cur_mode) {
    cur_mode = mode;
    drawBtn(30,0,30,16,Modes[mode].name,ST7735_BLACK,ST7735_DARKYELLOW);
  }

  uint8_t new_gc = trx.BandIndex < 0;
  if (new_gc != cur_gc) {
    drawBtn(60,0,30,16,"GC",ST7735_BLACK,(new_gc?(color_t)ST7735_DARKYELLOW:(color_t)ST7735_BLACK));
    cur_gc = new_gc;
  }
  
#ifdef RTC_ENABLE
  if (millis()-last_tmtm > 200) {
    RTCData d;
    char buf[12],*pb;
    last_tmtm=millis();
    RTC_Read(&d);   
    //sprintf(buf,"%2x:%02x",d.hour,d.min);
    pb=cwr_hex2sp(buf,d.hour);
    if (millis()/1000 & 1) *pb++=':';
    else *pb++=' ';
    pb=cwr_hex2(pb,d.min);
    *pb++=' ';
    //sprintf(buf,"%x.%02x.20",d.day,d.month);
    pb=cwr_hex2sp(pb,d.day);
    *pb++='/';
    pb=cwr_hex2(pb,d.month); 
    *pb=0;
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_CYAN,ST7735_BLACK);
    tft.setCursor(90,5);
    tft.print(buf);
  }
#endif
}

void Display_ST7735_SPI::clear()
{
  tft.fillScreen(ST7735_BLACK);
}

void Display_ST7735_SPI::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  (void)(fontsize); // supress warning about unused params
  tft.setFont(NULL);
  tft.setTextSize(1); // override and use always small font size
  tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
  tft.setCursor(0,0);
  if (title) {
    tft.println(title);
    tft.println("");
  }
  for (byte i=0; *items; items++,i++) {
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
