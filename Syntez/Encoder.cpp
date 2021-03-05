#include <Arduino.h>
#include "Encoder.h"
#include "config_hw.h"

/*
  подключение энкодера PIN 2,3
  AS5600 - I2C 3.3v
*/

#ifdef ENCODER_AS5600
#include <i2c.h>

#define AS5600_ADDR 0x36
#define RAWANGLEAddressMSB 0x0C
#define RAWANGLEAddressLSB 0x0D
#define STATUSAddress 0x0B

int last_angle = 0;
long Encoder_Value = 0;
long enc_last_tm = 0;
long enc_sum = 0;
uint8_t hi_mode = 0;

void Encoder::SetValue(long Value)
{
  Encoder_Value = Value;
}

long Encoder::GetDelta()
{
  int val;

  i2c_begin_write(AS5600_ADDR);
  i2c_write(RAWANGLEAddressMSB);
  i2c_begin_read(AS5600_ADDR);
  val = (int)(i2c_read() & 0xF) << 8; // 12 bits
  i2c_end();

  i2c_begin_write(AS5600_ADDR);
  i2c_write(RAWANGLEAddressLSB);
  i2c_begin_read(AS5600_ADDR);
  val |= (int)i2c_read();
  i2c_end();

  val >>= 4; // 4096 --> 256 "clicks" per turn

  //Serial.println(val); // debug
  int d1, d2;
  d1 = val - last_angle;
  if (val > last_angle) d2 = -(last_angle + 256 - val);
  else d2 = 256 - last_angle + val;
  long delta = (abs(d1) < abs(d2) ? d1 : d2);
  delta = delta * (hi_mode ? ENCODER_FREQ_HI_STEP : ENCODER_FREQ_LO_STEP) / 256;
  Encoder_Value += delta;
  delta = Encoder_Value;
  Encoder_Value = 0;
  last_angle = val;

  enc_sum += delta;
  if (millis()-enc_last_tm > 250) {
    if (enc_sum < 0) enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH/4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  return delta;
}

void Encoder::Setup()
{
}

#endif

#ifdef ENCODER_OPTICAL

#define ENC_LO_STEP (ENCODER_FREQ_LO_STEP / ENCODER_PULSE_PER_TURN)
#define ENC_HI_STEP (ENCODER_FREQ_HI_STEP / ENCODER_PULSE_PER_TURN)

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
  switch (state)
  {
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
  if (millis() - enc_last_tm > 250)
  {
    if (enc_sum < 0)
      enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH / 4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  sei(); //restart interrupts
}

void Encoder::Setup()
{
  cli();
  pinMode(2, INPUT_PULLUP);                   // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(3, INPUT_PULLUP);                   // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, EncoderHandler, CHANGE); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, EncoderHandler, CHANGE); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  sei();
}

#endif

#ifdef ENCODER_MECHANIC

#define ENC_LO_STEP (ENCODER_FREQ_LO_STEP / ENCODER_PULSE_PER_TURN / 4)
#define ENC_HI_STEP (ENCODER_FREQ_HI_STEP / ENCODER_PULSE_PER_TURN / 4)

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
  switch (state)
  {
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
  }
  long val;
  val = Encoder_Value;
  Encoder_Value = 0;
  enc_sum += val;
  if (millis() - enc_last_tm > 250)
  {
    if (enc_sum < 0)
      enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH / 4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  return val;
}

void Encoder::Setup()
{
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

#endif
