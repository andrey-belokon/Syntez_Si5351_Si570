#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

/*
  Так как энкодер может быть только один то все описываем глобальное
1. работает по прерываниям
2. автоматически переключает приращение частоты

подключение энкодера - GREEN -> pin 2; WHITE -> pin 3
!!!!! не изменять пины - завязано на прерывания !!!!!
*/

#define ENC_LO_STEP           (ENCODER_FREQ_LO_STEP/ENCODER_PULSE_PER_TURN)
#define ENC_HI_STEP           (ENCODER_FREQ_HI_STEP/ENCODER_PULSE_PER_TURN)
#define ENC_HILO_TRASHOLD_TM  ((1000000/ENCODER_PULSE_PER_TURN)*ENCODER_FREQ_LO_STEP/ENCODER_FREQ_HI_LO_TRASH)

volatile uint8_t enc_FlagA = 0;
volatile uint8_t enc_FlagB = 0;
volatile long Encoder_Value = 0;
volatile long enc_last_tm = 0;

void Encoder_SetValue(long Value) {
  cli();
  enc_last_tm = 0;
  Encoder_Value = Value;
  sei();
}

long Encoder_GetDelta() {
  long val;
  cli();
  val = Encoder_Value;
  Encoder_Value = 0;
  sei();
  return val;
}

void EncoderPinA(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && enc_FlagA) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    long tm = micros();
    if (tm-enc_last_tm < ENC_HILO_TRASHOLD_TM)
      Encoder_Value -= ENC_HI_STEP;
    else
      Encoder_Value -= ENC_LO_STEP;
    enc_last_tm = tm;
    enc_FlagB = enc_FlagA = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) enc_FlagB = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void EncoderPinB(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && enc_FlagB) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    long tm = micros();
    if (tm-enc_last_tm < ENC_HILO_TRASHOLD_TM)
      Encoder_Value += ENC_HI_STEP;
    else
      Encoder_Value += ENC_LO_STEP;
    enc_last_tm = tm;
    enc_FlagB = enc_FlagA = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) enc_FlagA = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void Encoder_Setup() {
  cli(); 
  pinMode(2, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(3, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,EncoderPinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,EncoderPinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  sei();
}

#endif
