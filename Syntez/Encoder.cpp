#include <Arduino.h>
#include "Encoder.h"
#include "config_hw.h"

/*
  подключение энкодера PIN 2,3
*/

#ifdef ENCODER_OPTICAL

#define ENC_LO_STEP           (ENCODER_FREQ_LO_STEP/ENCODER_PULSE_PER_TURN)
#define ENC_HI_STEP           (ENCODER_FREQ_HI_STEP/ENCODER_PULSE_PER_TURN)

volatile long Encoder_Value = 0;
volatile uint8_t enc_last = 0;
volatile long enc_last_tm = 0;
volatile long enc_sum = 0;
volatile uint8_t hi_mode = 0;

void Encoder::SetValue(long Value) 
{
  cli();
  Encoder_Value = Value;
  sei();
}

long Encoder::GetDelta() 
{
  long val;
  cli();
  val = Encoder_Value;
  Encoder_Value = 0;
  sei();
  return val;
}

void EncoderHandler() 
{
  cli(); //stop interrupts happening before we read pin values
  byte state = (PIND & 0xC) | enc_last;
  int delta;
  enc_last = state >> 2;
  switch(state){
    case 0b0100:
      delta = (hi_mode ? ENC_HI_STEP : ENC_LO_STEP);
      break;
    case 0b1110:
      delta = (hi_mode ? -ENC_HI_STEP : -ENC_LO_STEP);
      break;
    default:
      delta = 0;
  }
  Encoder_Value += delta;
  enc_sum += delta;
  if (millis()-enc_last_tm > 250) {
    if (enc_sum < 0) enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH/4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  sei(); //restart interrupts
}

void Encoder::Setup() {
  cli(); 
  pinMode(2, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(3, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,EncoderHandler,CHANGE); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,EncoderHandler,CHANGE); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  sei();
}

#else

#define ENC_LO_STEP           (ENCODER_FREQ_LO_STEP/ENCODER_PULSE_PER_TURN/4)
#define ENC_HI_STEP           (ENCODER_FREQ_HI_STEP/ENCODER_PULSE_PER_TURN/4)

long Encoder_Value = 0;
uint8_t enc_last = 0;
long enc_last_tm = 0;
long enc_sum = 0;
uint8_t hi_mode = 0;

void Encoder::SetValue(long Value) 
{
  Encoder_Value = Value;
}

long Encoder::GetDelta() 
{
  // обрабатываем все возможные состояния для увеличения кол-ва импульсов на оборот до 4х
  byte state = (PIND & 0xC) | enc_last;
  enc_last = state >> 2;
  switch(state){
    case 0b0100:
    case 0b1101:
    case 0b1011:
    case 0b0010:
      Encoder_Value -= (hi_mode ? ENC_HI_STEP : ENC_LO_STEP);
      break;
    case 0b1000:
    case 0b1110:
    case 0b0111:
    case 0b0001:
      Encoder_Value += (hi_mode ? ENC_HI_STEP : ENC_LO_STEP);
      break;
    default:
      return 0;
  }
  long val;
  val = Encoder_Value;
  Encoder_Value = 0;
  enc_sum += val;
  if (millis()-enc_last_tm > 250) {
    if (enc_sum < 0) enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH/4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  return val;
}

void Encoder::Setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

#endif
