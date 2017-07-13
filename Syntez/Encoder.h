#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

/*
1. работает по прерываниям
2. автоматически переключает приращение частоты

подключение энкодера - GREEN -> pin 2; WHITE -> pin 3
!!!!! не изменять пины - завязано на прерывания !!!!!
*/

class Encoder {
  public:
  	Encoder(
  	  int EncPulsePerTurn,    // кол-во пульсов энкодера на один оборот
  	  int StepLo,             // изменение частоты в Гц на один оборот в обычном режиме
  	  int StepHi,             // изменение частоты в Гц на один оборот в ускоренном режиме
  	  int HiLoStepTrashold    // порог переключения в ускоренный режим. если частота изменится более
  									          // чем на HiLoStepTrashold Гц за секунду то переходим в ускоренный режим
  	);
  
  	void setup();
  
  	void SetValue(long Value);
  	long GetValue();
  	long GetDelta();
};

#endif
