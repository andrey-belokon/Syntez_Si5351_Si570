#include <Arduino.h>
#include "utils.h"

char *cwr_str(char *p, const char *v)
{
  while ((*p++=*v++) != 0) ;
  return p-1;
}

char *cwr_byte(char *p, uint8_t v)
{
  char buf[4];
  uint8_t i;
  for(i=3; v > 0; v/=10) buf[i--] = v%10+0x30;
  for (i++; i <= 3; i++) *p++ = buf[i];
  return p;
}

char *cwr_int(char *p, int v)
{
  char buf[6];
  uint8_t i;
  for(i=5; v > 0; v/=10) buf[i--] = v%10+0x30;
  for (i++; i <= 5; i++) *p++ = buf[i];
  return p;
}

char *cwr_long(char *p, long v)
{
  char buf[12];
  uint8_t i;
  for(i=11; v > 0; v/=10) buf[i--] = v%10+0x30;
  for (i++; i <= 11; i++) *p++ = buf[i];
  return p;
}

char *cwr_hex2(char *p, uint8_t v)
{
  p[0] = ((v >> 4) & 0xF) + 0x30;
  p[1] = (v & 0xF) + 0x30;
  return p+2;
}

char *cwr_hex2sp(char *p, uint8_t v)
{
  p[1] = (v & 0xF) + 0x30;
  if ((v >>= 4) != 0)
  {
    p[0] = (v & 0xF) + 0x30;
  } else {
    p[0] = ' ';
  }
  return p+2;
}

void ltoazp(char *buf, long v, uint8_t n)
{
  buf+=n;
  for (; n > 0; n--, v/=10) 
    *--buf = v%10 + 0x30;
}

long atoln(char *buf, uint8_t n)
{
  long v = 0;
  for (; n > 0; n--) 
    v = v*10 + (*buf++) - 0x30;
  return v;
}
