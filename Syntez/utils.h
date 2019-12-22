#ifndef UTILS_H
#define UTILS_H

char *cwr_byte(char *p, uint8_t v);
char *cwr_int(char *p, int v);
char *cwr_long(char *p, long v);
char *cwr_hex2(char *p, uint8_t v);
char *cwr_hex2sp(char *p, uint8_t v);
char *cwr_str(char *p, const char *v);

// with zero leading fixed space
void ltoazp(char *buf, long v, uint8_t n);
long atoln(char *buf, uint8_t n);

#endif
