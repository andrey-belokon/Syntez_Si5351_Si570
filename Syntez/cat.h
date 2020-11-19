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
          if (trx.BandIndex < 0) {
            trx.state.VFO[i] = freq;
          } else {
            trx.ExecCommand(cmdHam);
            trx.state.VFO[i] = freq;
            trx.ExecCommand(cmdHam);
          }
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
/*      } else if (CAT_buf[0] == 'T' && CAT_buf[1] == 'X') {
       digitalWrite(PIN_OUT_TX,HIGH);
      } else if (CAT_buf[0] == 'R' && CAT_buf[1] == 'X') {
       digitalWrite(PIN_OUT_TX,LOW); */
      } else {
        Serial.write("?;");
      }
      CAT_buf_idx = 0;
    }
  }
}
