#include <i2c.h>
#include "pins.h"

uint8_t InputPullUpPin::Read() {
  if (pin == PIN_NC) return false;
  if (millis()-last_tm < 50) return last;
  uint8_t val = digitalRead(pin) == LOW;
  if (val != last) {
	  last = val;
	  last_tm = millis();
  }
  return val;
}

void InputPullUpPin::setup() {
  if (pin != PIN_NC) 
    pinMode(pin, INPUT_PULLUP); 
}

void InputAnalogPin::setup() {
  if (pin != PIN_NC) 
    pinMode(pin, INPUT); 
}

// read internal 1.1 source
int ReadV11Ref() 
{
  int acc = 0;

  analogReference(DEFAULT);
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(1); // otherwise unstable
  for (byte i=0; i < 3; i++) {
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both
    acc += (high<<8) | low;
  }
  return acc/3;
  delay(1);
}

int InputAnalogPin::Read() 
{
  int vref = ReadV11Ref(); 
  int new_value = 0;
  analogReference(DEFAULT);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value = (long)new_value *1100L/vref/3;
  if (abs(new_value-value) > rfac)
    value=new_value;
  return value;
}

int InputAnalogPin::ReadRaw()
{
  int new_value = 0;
  analogReference(DEFAULT);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value = new_value/3;
  if (abs(new_value-value) > rfac)
    value=new_value;
  return value;
}

void OutputBinPin::setup() 
{
  if (pin != PIN_NC) {
    pinMode(pin, OUTPUT);
    Write(def_value);
  }
}

void OutputBinPin::Write(uint8_t value) 
{
  if (pin != PIN_NC && state != value) {
	  digitalWrite(pin,value?(active_level == HIGH?HIGH:LOW):(active_level == HIGH?LOW:HIGH));
	  state = value;
  }
}

void OutputPCF8574::setup() 
{
  pcf8574_write(value);
}

void OutputPCF8574::Set(uint8_t pin, uint8_t state) 
{
  if (state) value |= (1 << (pin & 0x7));
  else value &= ~(1 << (pin & 0x7));
}

void OutputPCF8574::Write() 
{
  if (value != old_value) {
	  pcf8574_write(value);
	  old_value = value;
  }
}

void OutputPCF8574::pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
	i2c_write(data);
  i2c_end();
}

//////////////////////////////////////////////////////////////////////////////////

uint8_t OutputTone_state=0;
uint8_t OutputTone_pin=0;
uint8_t OutputTone_toggle=0;

void OutputTone(uint8_t pin, uint8_t value) 
{ 
  if (value) {
    if (!OutputTone_state) {
      OutputTone_pin = pin;
      pinMode(pin, OUTPUT);
      // init timer2 2kHz interrup
      cli();
      TCCR2A = 0;// set entire TCCR2A register to 0
      TCCR2B = 0;// same for TCCR2B
      TCNT2  = 0;//initialize counter value to 0
      // set compare match register for 2khz increments
      OCR2A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
      // turn on CTC mode
      TCCR2A |= (1 << WGM21);
      // Set CS21 and CS20 bits for 64 prescaler
      TCCR2B |= (1 << CS21) | (1 << CS20);   
      // enable timer compare interrupt
      TIMSK2 |= (1 << OCIE2A);
      sei();
      OutputTone_state = 1;
    }
  } else {
    if (OutputTone_state) {
      // disable interrup
      cli();
      TIMSK2 = 0;
      sei();
      // set pin to zero
      digitalWrite(pin,0);
      OutputTone_state = 0;
    }
  }
}

ISR(TIMER2_COMPA_vect)
{
  digitalWrite(OutputTone_pin, OutputTone_toggle++ & 1);
}
