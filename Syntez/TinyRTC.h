#ifndef TINYRTC_H
#define TINYRTC_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

// all data in BCD format (hex as decimal)
typedef struct {
  uint8_t sec;     // 0-59
  uint8_t min;     // 0-59
  uint8_t hour;    // 1-23
  uint8_t dow;     // day of week 1-7
  uint8_t day;     // 1-28/29/30/31
  uint8_t month;   // 1-12
  uint8_t year;    // 0-99
} RTCData;

void RTC_Write(RTCData* data);

void RTC_Read(void *data, uint8_t start, uint8_t count);

bool RTC_found();

#endif
