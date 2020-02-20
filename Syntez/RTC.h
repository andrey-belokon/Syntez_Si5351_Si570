#ifndef RTC_H
#define RTC_H

#include <Arduino.h>

// all data in BCD format (hex as decimal)
typedef struct {
  uint8_t sec;     // 0-59
  uint8_t min;     // 0-59
  uint8_t hour;    // 1-23
#ifdef RTC_DS1307
  uint8_t dow;     // day of week 1-7
  uint8_t day;     // 1-28/29/30/31
#else
  uint8_t day;     // 1-28/29/30/31
  uint8_t dow;     // day of week 1-7
#endif
  uint8_t month;   // 1-12
  uint8_t year;    // 0-99
} RTCData;

void RTC_Write(RTCData* data);
void RTC_Read(RTCData* data);

#endif
