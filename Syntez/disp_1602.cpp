#include "disp_1602.h"
#ifdef RTC_ENABLE
  #include "RTC.h"
#endif
#include "utils.h"

byte chInverseT[8] = { 0b11111, 0b11111, 0b11111, 0b10001, 0b11011, 0b11011, 0b11111, 0b11111};
byte chInverseX[8] = { 0b11111, 0b11111, 0b11111, 0b10101, 0b11011, 0b10101, 0b11111, 0b11111};
byte chLock[8] = { 0b00000, 0b01110, 0b10001, 0b11111, 0b10101, 0b10001, 0b11111, 0b00000};

byte SMetr10[8] = {0b11000,0b11000,0b11000,0b11000,0b11000,0b11000,0b11000,0b11000};
byte SMetr11[8] = {0b11011,0b11011,0b11011,0b11011,0b11011,0b11011,0b11011,0b11011};

void Display_1602_I2C::setup() {
  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея
  lcd.createChar(1, SMetr10);
  lcd.createChar(2, SMetr11);
  lcd.createChar(3, chInverseT);
  lcd.createChar(4, chInverseX);
  lcd.createChar(5, chLock);
}

void Display_1602_I2C::Draw(TRX& trx) {
  char buf[2][17];
  int vfo_idx = trx.GetVFOIndex();
  long f = (trx.state.VFO[vfo_idx]+5) / 10;

  memset(buf,' ',34);

  if (tx != trx.TX) {
    tx = trx.TX;
  }

  if (trx.TX) {
    buf[0][0] = (char)3;
    buf[0][1] = (char)4;
  } else {
    if (trx.Lock)
      buf[0][1] = (char)5;
  }

  if (trx.QRP)
    buf[0][2] = (char)0b01011100;

  buf[0][14] = '0'+f%10; f/=10;
  buf[0][13] = '0'+f%10; f/=10;
  buf[0][12] = '.';
  buf[0][11] = '0'+f%10; f/=10;
  buf[0][10] = '0'+f%10; f/=10;
  buf[0][9] = '0'+f%10; f/=10;
  buf[0][8] = '.';
  buf[0][7] = '0'+f%10; f/=10;
  if (f > 0) buf[0][6] = '0'+f;

  if (vfo_idx == 0) buf[0][15] = 'A';
  else buf[0][15] = 'B';

  switch (trx.state.AttPre) {
#ifdef MODE_SUPER21
    case 1:
      buf[0][3] = 'A';
	    buf[0][4] = '1';
      //buf[0][5] = 'T';
      break;
    case 2:
      buf[0][3] = 'A';
	    buf[0][4] = '2';
      //buf[0][5] = 'T';
      break;
    case 3:
#else
    case 1:
      buf[0][3] = 'A';
	    buf[0][4] = 'T';
      //buf[0][5] = 'T';
      break;
    case 2:
#endif
      buf[0][3] = 'P';
      buf[0][4] = 'R';
      //buf[0][5] = 'E';
      break;
  }

  const char *p = Modes[trx.state.mode].name;
  buf[1][7] = *p++;
  if (*p) {
    buf[1][8] = *p++;  
    if (*p) {
      buf[1][9] = *p++;  
    }
  }

  if (trx.state.Split && trx.RIT) {
    if ((millis() / 700) & 1) {
      buf[1][12] = 'S';
      buf[1][13] = 'P';
      buf[1][14] = 'L';
    } else {
      buf[1][12] = 'R';
      buf[1][13] = 'I';
      buf[1][14] = 'T';
    }
  } else if (trx.state.Split) {
    buf[1][12] = 'S';
    buf[1][13] = 'P';
    buf[1][14] = 'L';
  } else if (trx.RIT) {
    buf[1][12] = 'R';
    buf[1][13] = 'I';
    buf[1][14] = 'T';
  } else {
#ifdef RTC_ENABLE
    static long last_tmtm=0;
    static RTCData d;
    if (millis()-last_tmtm > 200) {
      RTC_Read(&d);
      last_tmtm=millis();
    }
    char *pb;
    //sprintf(buf,"%2x:%02x",d.hour,d.min);
    pb=cwr_hex2sp(buf[1]+11,d.hour);
    if (millis()/1000 & 1) *pb++=':';
    else *pb++=' ';
    pb=cwr_hex2(pb,d.min);
#endif
  }

  // S-meter
//  switch (millis()/200 & 0xF) { // debug
  switch (trx.SMeter) {
    case 14:
    case 15:
      buf[1][6] = (char)1;
    case 12:
    case 13:
      buf[1][5] = (char)2;
      buf[1][4] = (char)2;
      buf[1][3] = (char)2;
      buf[1][2] = (char)2;
      buf[1][1] = (char)2;
      buf[1][0] = (char)2;
      break;
    case 11:
      buf[1][5] = (char)1;
    case 10:
      buf[1][4] = (char)2;
      buf[1][3] = (char)2;
      buf[1][2] = (char)2;
      buf[1][1] = (char)2;
      buf[1][0] = (char)2;
      break;
    case 9:
      buf[1][4] = (char)1;
    case 8:
      buf[1][3] = (char)2;
      buf[1][2] = (char)2;
      buf[1][1] = (char)2;
      buf[1][0] = (char)2;
      break;
    case 7:
      buf[1][3] = (char)1;
    case 6:
      buf[1][2] = (char)2;
      buf[1][1] = (char)2;
      buf[1][0] = (char)2;
      break;
    case 5:
      buf[1][2] = (char)1;
    case 4:
      buf[1][1] = (char)2;
      buf[1][0] = (char)2;
      break;
    case 3:
      buf[1][1] = (char)1;
    case 2:
      buf[1][0] = (char)2;
      break;
    case 1:
      buf[1][0] = (char)1;
      break;
  }

  buf[0][16] = 0; // stop for .print
  buf[1][16] = 0; // stop for .print
  lcd.setCursor(0, 0);
  lcd.print(buf[0]);
  lcd.setCursor(0, 1);
  lcd.print(buf[1]);
}

void Display_1602_I2C::DrawMenu(const char* title, const char** items, uint8_t selected, const char* help, uint8_t fontsize)
{
  char buf[2][17];

  (void)(help); // supress warning about unused params
  (void)(fontsize); // supress warning about unused params
  memset(buf,' ',34);
  strncpy(buf[0],title,16);
  //sprintf(buf[1],">%s",items[selected]);
  buf[1][0]='>';
  strncpy(buf[1]+1,items[selected],15);
  // supress null writed by sprintf
  for (int i=2;i < 16;i++) {
    if (buf[1][i] == 0) buf[1][i]=' ';
  }
  buf[0][16] = 0; // stop for .print
  buf[1][16] = 0; // stop for .print
  lcd.setCursor(0, 0);
  lcd.print(buf[0]);
  lcd.setCursor(0, 1);
  lcd.print(buf[1]);
}
