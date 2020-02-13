#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
  public:
  static void Setup();
  static long GetDelta();
  static void SetValue(long Value);
};

#endif
