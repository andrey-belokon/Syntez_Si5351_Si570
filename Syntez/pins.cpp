#include "i2c.h"
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

// code from http://tim4dev.com/arduino-secret-true-voltmeter/
// rewriten to non-float style
int ReadV11Ref() 
{
  int acc = 0;

  analogReference(DEFAULT);
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
      ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
      ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
      // works on an Arduino 168 or 328
      ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(3); // Wait for Vref to settle

  for (byte i=0; i < 3; i++) {
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both
    acc += (high<<8) | low;
    delay(3);
  }
  return acc/3;
}

int InputAnalogPin::Read() 
{
  int vref = ReadV11Ref(); 
  //int vref = 1100L*1024L/5000L; debug
  analogReference(DEFAULT);
  int new_value = 0;
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
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value = new_value/3;
  if (abs(new_value-value) > rfac)
    value=new_value;
  return value;
}

void OutputTonePin::setup() {
  if (pin != PIN_NC) 
    pinMode(pin, OUTPUT); 
}

void OutputTonePin::Write(uint8_t value) { 
  if (pin != PIN_NC) {
    if (value) tone(pin,freq); 
    else noTone(pin); 
  }
}

void OutputBinPin::setup() {
  if (pin != PIN_NC) {
	  pinMode(pin, OUTPUT);
	  Write(def_value);
  }
}

void OutputBinPin::Write(uint8_t value) {
  if (pin != PIN_NC && state != value) {
	  digitalWrite(pin,value?(active_level == HIGH?HIGH:LOW):(active_level == HIGH?LOW:HIGH));
	  state = value;
  }
}

void OutputPCF8574::setup() {
  pcf8574_write(value);
}

void OutputPCF8574::Set(uint8_t pin, uint8_t state) {
  if (state) value |= (1 << (pin & 0x7));
  else value &= ~(1 << (pin & 0x7));
}

void OutputPCF8574::Write() {
  if (value != old_value) {
	  pcf8574_write(value);
	  old_value = value;
  }
}

void OutputPCF8574::pcf8574_write(uint8_t data) {
  if (i2c_addr >= 0) {
	  i2c_begin_write(i2c_addr);
	  i2c_write(data);
    i2c_end();
  }
}


