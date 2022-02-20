#ifdef CAT_PROTOCOL_KENWOOD_TS480
#define CAT_DEFINED

#define CAT_BUF_SIZE  40
char CAT_buf[CAT_BUF_SIZE];
uint8_t CAT_buf_idx = 0;

void ExecCAT()
{
  int b;
  while ((b = Serial.read()) >= 0) {
  	if (b <= ' ') continue;
    if (CAT_buf_idx >= CAT_BUF_SIZE) CAT_buf_idx = 0;
    CAT_buf[CAT_buf_idx++] = (uint8_t)b;
    if (b == ';') {
      // parse command
      if (CAT_buf[0] == 'I' && CAT_buf[1] == 'F') {
        ltoazp(CAT_buf + 2, trx.state.VFO[trx.state.VFO_Index], 11);
        memset(CAT_buf + 13, ' ', 5);
        if (trx.RIT) {
          ltoazp(CAT_buf + 18, trx.RIT_Value, 5);
          CAT_buf[23] = '1';
        } else {
          memset(CAT_buf + 18, '0', 6);
        }
        memset(CAT_buf + 24, '0', 4);
        CAT_buf[28] = '0' + (trx.TX & 1);
        CAT_buf[29] = Modes[trx.state.mode].cat_name;
        CAT_buf[30] = '0' + trx.state.VFO_Index;
        CAT_buf[31] = '0';
        CAT_buf[32] = '0' + (trx.state.Split & 1);
        memset(CAT_buf + 33, '0', 3);
        CAT_buf[36] = ' ';
        CAT_buf[37] = ';';
        CAT_buf[38] = 0;
        Serial.write(CAT_buf);
      } else if (CAT_buf[0] == 'I' && CAT_buf[1] == 'D') {
       CAT_buf[2] = '0';
       CAT_buf[3] = '2';
       CAT_buf[4] = '0';
       CAT_buf[5] = ';';
       CAT_buf[6] = 0;
       Serial.write(CAT_buf);
      } else if (CAT_buf[0] == 'F' && (CAT_buf[1] == 'A' || CAT_buf[1] == 'B')) {
        uint8_t i = CAT_buf[1] - 'A';
        if (CAT_buf[2] == ';') {
          ltoazp(CAT_buf + 2, trx.state.VFO[i], 11);
          CAT_buf[13] = ';';
          CAT_buf[14] = 0;
          Serial.write(CAT_buf);
        } else {
          long freq = atoln(CAT_buf + 2, 11);
          if (trx.BandIndex < 0) trx.state.VFO[i] = freq;
          else trx.SetFreq(freq,i);
        }
      } else if (CAT_buf[0] == 'M' && CAT_buf[1] == 'D') {
        if (CAT_buf[2] == ';') {
          CAT_buf[2] = Modes[trx.state.mode].cat_name;
          CAT_buf[3] = ';';
          CAT_buf[4] = 0;
          Serial.write(CAT_buf);
        } else {
          for (uint8_t i=0; Modes[i].cat_name != 0; i++) {
            if (Modes[i].cat_name == CAT_buf[2]) {
              trx.state.mode = i;
              break;
            }
          }
        }
      } else if (CAT_buf[0] == 'B' && CAT_buf[1] == 'D') {
        trx.ExecCommand(cmdBandDown);
      } else if (CAT_buf[0] == 'B' && CAT_buf[1] == 'U') {
        trx.ExecCommand(cmdBandUp);
      } else if (CAT_buf[0] == 'V' && CAT_buf[1] == 'V') {
        trx.ExecCommand(cmdVFOEQ);
      } else if (CAT_buf[0] == 'T' && CAT_buf[1] == 'X') {
        trx.CATTX = true;
      } else if (CAT_buf[0] == 'R' && CAT_buf[1] == 'X') {
        trx.CATTX = false;
      } else {
        Serial.write("?;");
      }
      CAT_buf_idx = 0;
    }
  }
}

#endif

#ifdef CAT_PROTOCOL_YAESU_FT817
#define CAT_DEFINED

typedef enum
{
  FT817_LOCK_ON       = 0x00,
  FT817_LOCK_OFF      = 0x80,
  FT817_PTT_ON        = 0x08,
  FT817_PTT_OFF       = 0x88,
  FT817_SET_FREQ      = 0x01,
  FT817_MODE_SET      = 0x07,
//  FT817_RIT_ON        = 0x05,
//  FT817_RIT_OFF       = 0x85,
//  FT817_RIT_OFFSET    = 0xF5,
  FT817_TOGGLE_VFO    = 0x81,
  FT817_SPLIT_ON      = 0x02,
  FT817_SPLIT_OFF     = 0x82,
  FT817_READ_RX_STATE = 0xE7,
  FT817_READ_TX_STATE = 0xF7,
  FT817_GET_FREQ      = 0x03,
} FT817_COMMAND;

typedef enum
{
  FT817_LSB = 0x00,
  FT817_USB = 0x01,
  FT817_CW  = 0x02,  //CW-U
  FT817_CWR = 0x03,  //CW-L
  FT817_AM  = 0x04,
  FT817_FM  = 0x08,
  FT817_DIG = 0x0A,
  FT817_PKT = 0x0C
} FT817_MODE;

#define CAT_BUF_SIZE  4
char CAT_buf[CAT_BUF_SIZE];
uint8_t CAT_buf_idx = 0;

void ExecCAT()
{
  int cmd;
  while ((cmd = Serial.read()) >= 0) {
    if (CAT_buf_idx >= CAT_BUF_SIZE) {
      // получили 5й байт - команду. выполняем
      long freq;
      byte reply[5];
      byte reply_len = 1;
      reply[0] = 0;
      CAT_buf_idx=0;
      switch (cmd) {
        case FT817_GET_FREQ:
          freq = trx.state.VFO[trx.state.VFO_Index];
          for (byte i = 3; ; i--) {
            freq /= 10;
            reply[i] = freq % 10;
            freq /= 10;
            reply[i] |= (freq % 10) << 4;
            if (i == 0) break;
          }
          reply[4] = trx.state.mode;
          reply_len = 5;
          break;

        case FT817_SET_FREQ:
          freq = 0;
          for (byte i = 0; i < 4; i++) {
            freq *= 100;
            freq += (CAT_buf[i] >> 4) * 10 + (CAT_buf[i] & 0x0F);
          }
          freq *= 10;
          trx.SetFreq(freq,trx.state.VFO_Index);
          break;

        case FT817_TOGGLE_VFO:
          trx.ExecCommand(cmdVFOSel);
          break;

        case FT817_MODE_SET:
          switch (CAT_buf[0]) {
            case FT817_LSB:
              trx.state.mode = MODE_LSB;
              break;
            case FT817_USB:
              trx.state.mode = MODE_USB;
              break;
            case FT817_CWR:
            case FT817_CW:
              trx.state.mode = MODE_CW;
              break;
          }
          break;
        
        case FT817_PTT_ON:
          trx.CATTX = 1;
          break;

        case FT817_PTT_OFF:
          trx.CATTX = 0;
          break;

        case FT817_LOCK_ON:
          trx.Lock = 1;
          break;

        case FT817_LOCK_OFF:
          trx.Lock = 0;
          break;

        case FT817_SPLIT_ON:
          trx.state.Split = 1;
          break;

        case FT817_SPLIT_OFF:
          trx.state.Split = 0;
          break;
        
        case FT817_READ_RX_STATE:
          reply[0] = trx.SMeter;
          break;

        case FT817_READ_TX_STATE:
          reply[0] = 0xFF;
          if (trx.TX) reply[0] &= 0x7F;
          if (trx.state.Split) reply[0] &= 0xDF;
          break;
      }
      Serial.write(reply,reply_len);
    } else {
      CAT_buf[CAT_buf_idx++] = (uint8_t)cmd;
    }
  }
}

#endif

#ifndef CAT_DEFINED
void ExecCAT()
{ /* not CAT support */ }
#endif
