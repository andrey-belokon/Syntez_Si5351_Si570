#ifndef DISP_ILI9341_H
#define DISP_ILI9341_H

#include <Arduino.h>
#include "TRX.h"

class Display_ILI9341_SPI: public TRXDisplay {
  private:
    uint8_t tx;
  public:
	  void setup();
    void reset();
	  void Draw(TRX& trx);
	  void clear();
	  void DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize);
};

#include "TinyRTC.h"
#include <SPI.h>        // must include this here (or else IDE can't find it)                                         
#include "utils.h"

#define  ILI9341_CS_PIN    10      // <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define ILI9341_DC_PIN    9     // <= DC pin (1=data or 0=command indicator line) also called RS
//#define ILI9341_RST_PIN   8     // <= RST pin (optional) - not used
#define ILI9341_SAVE_SPCR 0     // <= 0/1 with 1 to save/restore AVR SPI control register (to "play nice" when other SPI use)

#include <PDQ_GFX.h>        // PDQ: Core graphics library
#include <PDQ_ILI9341.h>      // PDQ: Hardware-specific driver library

#ifdef GFX_FONT_PACKED
#include "font\Tahoma18.pck.h"
#else
#include "font\Tahoma18.h"
#endif

#define color_rgb(r,g,b) (((r)<<11)|((g)<<5)|(b))
#define ILI9341_DARKGRAY color_rgb(0x3,0x7,0x3)
#define ILI9341_GRAY color_rgb(0x7,0xf,0x7)
#define ILI9341_CYAN color_rgb(0x3,0x1F,0x9)
#define ILI9341_DARKYELLOW color_rgb(0xf,0x1f,0x0)

PDQ_ILI9341 tft;

long cur_freq=0;
long cur_freq2=0;
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

void Display_ILI9341_SPI::setup() 
{
  tft.begin();
#ifdef TFT_ORIENTATION
  tft.setRotation((TFT_ORIENTATION << 1)+1);
#else
  tft.setRotation(3);
#endif  
  tft.fillScreen(ILI9341_BLACK);
}

void Display_ILI9341_SPI::reset()
{
  cur_freq=0;
  cur_freq2=0;
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

/* Routine to Draw Large 7-Segment formated number with Arduino TFT Library
   Contributed by William Zaggle  (Uses TFT Library DrawLine and Fill Rectangle functions).
   https://forum.arduino.cc/index.php?topic=307348.0

   int n - The number to be displayed
   int xLoc = The x location of the upper left corner of the number
   int yLoc = The y location of the upper left corner of the number
   int cS = The size of the number.
   fC is the foreground color of the number
   bC is the background color of the number (prevents having to clear previous space)
   nD is the number of digit spaces to occupy (must include space for minus sign for numbers < 0).

    // Example to draw the number 37 in four directions in four corners of the display
    draw7Number(37,10,10,4, ILI9341_WHITE , ILI9341_BLACK,2);          //LEFT2RIGHT
    draw7Number90(37,10,310,4, ILI9341_WHITE , ILI9341_BLACK,2);     //DOWN2UP
    draw7Number180(37,230,310,4, ILI9341_WHITE , ILI9341_BLACK,2);  //RIGHT2LEFT
    draw7Number270(37,230,10,4, ILI9341_WHITE , ILI9341_BLACK,2);    //UP2DOWN
*/
void draw7Number(int n, int xLoc, int yLoc, byte cS, color_t fC, int bC, byte nD) {
  int num=abs(n),i,s,t,w,col,h,a,b,si=0,j=1,d=0,S2=5*cS,S3=2*cS,S4=7*cS,x1=cS+1,x2=S3+S2+1,y1=yLoc+x1,y3=yLoc+S3+S4+1;
  int seg[7][3]={{x1,yLoc,1},{x2,y1,0},{x2,y3+x1,0},{x1,(2*y3)-yLoc,1},{0,y3+x1,0},{0,y1,0},{x1,y3,1}};
  byte nums[12]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x00,0x40},c=(c=abs(cS))>10?10:(c<1)?1:c,cnt=(cnt=abs(nD))>10?10:(cnt<1)?1:cnt;
  for (xLoc+=cnt*(d=S2+(3*S3)+2);cnt>0;cnt--){
    for (i=(num>9)?num%10:((!cnt)&&(n<0))?11:((nD<0)&&(!num))?10:num,xLoc-=d,num/=10,j=0;j<7;++j){
      col=(nums[i]&(1<<j))?fC:bC;
      if (seg[j][2])for(w=S2,t=seg[j][1]+S3,h=seg[j][1]+cS,a=xLoc+seg[j][0]+cS,b=seg[j][1];b<h;b++,a--,w+=2)tft.drawFastHLine(a,b,w,col);
      else for(w=S4,t=xLoc+seg[j][0]+S3,h=xLoc+seg[j][0]+cS,b=xLoc+seg[j][0],a=seg[j][1]+cS;b<h;b++,a--,w+=2)tft.drawFastVLine(b,a,w,col);
      for (;b<t;b++,a++,w-=2)seg[j][2]?tft.drawFastHLine(a,b,w,col):tft.drawFastVLine(b,a,w,col);
    }
  }
}

void drawFreq(int x, int y, long f, color_t c)
{
  const int DotWidth = 24;
  const int cS = 3;
  const int NumWidth = 9*cS+2;
  const int NumHeight = 20*cS+4;
  const int SpaceWidth = (NumWidth+3)/4;
  const int NumWidth2 = 9*(cS-1)+2;
  const int SpaceWidth2 = (NumWidth2+3)/4;
  const byte Space[7] = {NumWidth2+SpaceWidth2, NumWidth2+DotWidth, NumWidth+SpaceWidth, NumWidth+SpaceWidth, NumWidth+DotWidth, NumWidth+SpaceWidth, NumWidth+SpaceWidth};
  int xx=x+4*SpaceWidth+2*DotWidth+5*NumWidth+2*NumWidth2;
  int v;
  long f2 = cur_freq;
  for (byte i=0; i <= 6; i++) {
    v=f%10; f/=10;
    if (v != f2%10 || cur_freq == 0) {
      if (i <= 1) 
        draw7Number(v,xx,y,cS-1, c, ILI9341_BLACK, 1);
      else if (i == 6 && v == 0)
        draw7Number(v,xx,y,cS, ILI9341_BLACK, ILI9341_BLACK, 1);
      else
        draw7Number(v,xx,y,cS, c, ILI9341_BLACK, 1);
    }
    f2/=10;
    xx-=Space[i];
  }
}

void drawFreq2(int x, int y, long v, color_t c)
{
  char buf[10];
  buf[9] = 0;
  for (byte i=8; i > 0; ) {
    buf[i--] = '0'+v%10; 
    v/=10;
    if (i == 6 || i == 2) buf[i--] = '.';
  }
  if (v > 0) buf[0] = '0'+v;
  else buf[0] = ' ';
  tft.setTextColor(c,ILI9341_BLACK);
  tft.setFont(NULL);
  tft.setTextSize(2);
  tft.setCursor(x,y);
  tft.print(buf);
  tft.setTextSize(1);
}

void drawBtn(int x, int y, uint8_t w, uint8_t h, const char *title, color_t cframe, color_t ctext)
{
  if (cframe == ILI9341_BLACK)
  {
    tft.fillRect(x,y,w,h,ILI9341_BLACK);
  } else {
    //tft.fillRoundRect(x,y,w,h,5,cframe); // +400 byte of code
    tft.fillRect(x,y+5,w,h-2*5,cframe);
    for (uint8_t yy=5; yy >= 1; yy--) {
      tft.drawFastHLine(x+yy, y+5-yy, w-2*yy, cframe);
      tft.drawFastHLine(x+yy, y+h-5+yy-1, w-2*yy, cframe);
    }
    //tft.fillRect(x,y,w,h,cframe);
  }
  if (*title) {
    int16_t x1,y1;
    uint16_t w1,h1;
    tft.setTextColor(ctext);
    tft.setFont(&Tahoma18);
    tft.getTextBounds((char *)title,x,y,&x1,&y1,&w1,&h1);
    tft.setCursor(x+((w-w1)/2), y+((h-29)/2)+29);
    tft.print(title);
  }
}

void Display_ILI9341_SPI::Draw(TRX& trx) {
  const int FREQ_Y = 83;
  const int FREQ_H = 105; 
  const int BTN_Y = 204;
  const int SMetr_Y = 42;
  const int SMetr_H = 12;
  uint8_t vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+5) / 10;
  long f2 = (trx.state.VFO[vfo_idx^1]+5) / 10;

  tft.setTextSize(1);
  
  if (!init_smetr) {
    init_smetr=1;
    tft.setTextColor(ILI9341_GREEN);
    tft.setFont(NULL);
    tft.setCursor(8,SMetr_Y);
    tft.print("1");
    tft.setCursor(10+40,SMetr_Y);
    tft.print("3");
    tft.setCursor(10+40*2,SMetr_Y);
    tft.print("5");
    tft.setCursor(10+40*3+3,SMetr_Y);
    tft.print("7");
    tft.setCursor(10+40*4+3,SMetr_Y);
    tft.print("9");
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(10+40*5,SMetr_Y);
    tft.print("+20");
    tft.setCursor(10+40*6,SMetr_Y);
    tft.print("+40");
    tft.setCursor(10+40*7+3,SMetr_Y);
    tft.print("+60");
    for (int j=0; j < 15; j++) 
      tft.drawRect(j*21,SMetr_Y+12,18,SMetr_H,(j < 9 ? ILI9341_GREEN : ILI9341_RED));
    tft.drawFastHLine(0,BTN_Y-7,320,ILI9341_GRAY);
    tft.drawFastHLine(0,BTN_Y-5,320,ILI9341_GRAY);
  }
  
  for (int j=0; j < 15; j++) {
    if (j < trx.SMeter) {
      if (!cur_sm[j]) {
        tft.fillRect(j*21,SMetr_Y+12,18,SMetr_H,(j < 9 ? ILI9341_GREEN : ILI9341_RED));
        cur_sm[j]=1;
      }
    } else {
      if (cur_sm[j]) {
        tft.fillRect(j*21+1,SMetr_Y+12+1,18-2,SMetr_H-2,ILI9341_BLACK);
        cur_sm[j]=0;
      }
    }
  }

  if (cur_vfo_idx != vfo_idx) {
    cur_freq=0;
    cur_freq2=0;
//    cur_freq_buf[0]=0;
    cur_vfo_idx=vfo_idx;
    tft.fillRect(20,FREQ_Y,300,FREQ_H,ILI9341_BLACK);
    if (cur_rit) {
      tft.fillRect(10,(cur_vfo_idx ? FREQ_Y+82 : FREQ_Y+4),90,16,ILI9341_BLACK);
      cur_ritval=0xffff;
    }
  }
  
  if (f != cur_freq) {
    if (vfo_idx == 0)
      drawFreq(20,FREQ_Y,f,ILI9341_YELLOW);
    else
      drawFreq(20,FREQ_Y+40,f,ILI9341_YELLOW);
    cur_freq=f;
  }

  if (f2 != cur_freq2) {
    if (vfo_idx == 0)
      drawFreq2(200,FREQ_Y+82,f2,ILI9341_CYAN);
    else
      drawFreq2(200,FREQ_Y+4,f2,ILI9341_CYAN);
    cur_freq2=f2;
  }

  if (trx.state.AttPre != cur_attpre) {
    switch (cur_attpre=trx.state.AttPre) {
      case 0:
        drawBtn(0,BTN_Y,66,36,"ATT",ILI9341_BLACK,ILI9341_DARKGRAY);
        break;
      case 1:
        drawBtn(0,BTN_Y,66,36,"ATT",ILI9341_BLUE,ILI9341_WHITE);
        break;
      case 2:
        drawBtn(0,BTN_Y,66,36,"PRE",ILI9341_GREEN,ILI9341_GRAY);
        break;
    }  
  }

  if (trx.QRP != cur_qrp) {
    if (cur_qrp=trx.QRP) 
      drawBtn(85,BTN_Y,66,36,"QRP",ILI9341_BLUE,ILI9341_WHITE);
    else
      drawBtn(85,BTN_Y,66,36,"QRP",ILI9341_BLACK,ILI9341_DARKGRAY);
  }
  
  if (trx.RIT != cur_rit) {
    if (cur_rit=trx.RIT) {
      drawBtn(169,BTN_Y,66,36,"RIT",ILI9341_BLUE,ILI9341_WHITE);
      cur_ritval=0xffff;
    } else {
      drawBtn(169,BTN_Y,66,36,"RIT",ILI9341_BLACK,ILI9341_DARKGRAY);
      tft.fillRect(10,(cur_vfo_idx ? FREQ_Y+4 : FREQ_Y+82),90,16,ILI9341_BLACK);
    }
  }
  
  if (cur_rit && trx.RIT_Value != cur_ritval) {
    int y,v=trx.RIT_Value;
    char buf[14];
    if (cur_vfo_idx == 0) y=FREQ_Y+82;
    else y=FREQ_Y+4;
    cur_ritval=trx.RIT_Value;
    if (v == 0) {
      cwr_str(buf,"0    ");
    } else {
      if (v < 0) {
        buf[0]='-';
        v=-v;
      }
      else buf[0]='+'; 
      cwr_str(cwr_int(buf+1,v),"Hz   ");
    }
    tft.setFont(NULL);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLUE,ILI9341_BLACK);
    tft.setCursor(10,y);
    tft.print(buf);
    tft.setTextSize(1);
  }
  
  if (trx.state.Split != cur_split) {
    if (cur_split=trx.state.Split)
      drawBtn(254,BTN_Y,66,36,"SPL",ILI9341_BLUE,ILI9341_WHITE);
    else
      drawBtn(254,BTN_Y,66,36,"SPL",ILI9341_BLACK,ILI9341_DARKGRAY);
  }

  if (trx.Lock != cur_lock) {
    cur_lock=trx.Lock;
    if (cur_lock)
      tft.setTextColor(ILI9341_RED);
    else
      tft.setTextColor(ILI9341_BLACK);
    tft.setFont(&Tahoma18);
    tft.setCursor(140,33);
    tft.print("#");
  }

  if (trx.TX != cur_tx) {
    if (cur_tx=trx.TX) 
      drawBtn(0,0,40,36,"TX",ILI9341_RED,ILI9341_YELLOW);
    else
      drawBtn(0,0,40,36,"RX",ILI9341_BLACK,ILI9341_GREEN);
  }

  uint8_t sb = trx.state.sideband;
  if (trx.BandIndex >= 0 && Bands[trx.BandIndex].sideband != trx.state.sideband) sb |= 0x80;
  if (sb != cur_sideband) {
    char *sb_txt = (trx.state.sideband == LSB ? "LSB" : "USB");
    cur_sideband=sb;
    if (sb & 0x80)
      drawBtn(160,0,50,36,sb_txt,ILI9341_RED,ILI9341_YELLOW);
    else
      drawBtn(160,0,50,36,sb_txt,ILI9341_BLACK,ILI9341_BLUE);
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
      drawBtn(40,0,56,36,buf,ILI9341_BLACK,ILI9341_BLUE);
    } else
      drawBtn(40,0,56,36,"",ILI9341_BLACK,ILI9341_BLUE);
  }

  uint8_t cw=trx.inCW();
  if (cw != cur_cw) {
    if (cur_cw=cw)
      drawBtn(90,0,50,36,"CW",ILI9341_BLACK,ILI9341_DARKYELLOW);
    else
      drawBtn(90,0,50,36,"",ILI9341_BLACK,ILI9341_DARKYELLOW);
  }

  if (RTC_found() && millis()-last_tmtm > 200) {
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
    tft.setTextColor(ILI9341_CYAN,ILI9341_BLACK);
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
}

void Display_ILI9341_SPI::clear()
{
  tft.fillScreen(ILI9341_BLACK);
}

void Display_ILI9341_SPI::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  tft.setFont(NULL);
  tft.setTextSize(fontsize);
  tft.setTextColor(ILI9341_YELLOW,ILI9341_BLACK);
  tft.setCursor(0,0);
  if (title) {
    tft.print("    ");
    tft.println(title);
  }
  for (byte i=0; *items; items++,i++) {
    if (i == selected) tft.print(" >");
    else tft.print("  ");
    tft.print(*items);
    tft.println("   ");
  }
  if (help) {
    tft.println("");
    tft.println(help);
  }
}

#endif
