#include "Encoder.h"

/*
  Так как энкодер может быть только один то все описываем глобальное
*/

int LO_STEP;
int HI_STEP;
int HILO_TRASHOLD_TM;
// подключение энкодера - GREEN -> pin 2; WHITE -> pin 3
// !!!!! не изменять пины - завязано на прерывания !!!!!
#define pinA 2
#define pinB 3
volatile uint8_t aFlag = 0;
volatile uint8_t bFlag = 0;
volatile long EncoderValue = 0;
volatile unsigned long last_tm = 0;

Encoder::Encoder(int EncPulsePerTurn, int StepLo, int StepHi, int HiLoStepTrashold) {
    LO_STEP = StepLo/EncPulsePerTurn;
    HI_STEP = StepHi/EncPulsePerTurn;
    HILO_TRASHOLD_TM = (1000000/EncPulsePerTurn)*StepLo/HiLoStepTrashold;
}

void Encoder::SetValue(long Value) {
  last_tm = 0;
  EncoderValue = Value;
}

long Encoder::GetValue() {
  return EncoderValue;
}

long Encoder::GetDelta() {
  long val = EncoderValue;
  EncoderValue = 0;
  return val;
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    long tm = micros();
    if (tm-last_tm < HILO_TRASHOLD_TM)
      EncoderValue -= HI_STEP;
    else
      EncoderValue -= LO_STEP;
    last_tm = tm;
    bFlag = aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    long tm = micros();
    if (tm-last_tm < HILO_TRASHOLD_TM)
      EncoderValue += HI_STEP;
    else
      EncoderValue += LO_STEP;
    last_tm = tm;
    bFlag = aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void Encoder::setup() {
  cli(); 
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  sei();
}

