#ifndef KEYPAD6I2C_H
#define KEYPAD6I2C_H

#include <Arduino.h>

/*
 * 6-buttons keypad on PCF8574
 * button connected to P0..P5
 */

class Keypad_6_I2C {
  private:
    static uint8_t last_code;
    static uint8_t FnPressed, KeyPressed;
  public:
	  Keypad_6_I2C(uint8_t _i2c_addr);
    
    static void setup();
    
    // возвращает код нажатой клавиши
    // cmdNone если ничего не нажато. подавляет дребезг
    static uint8_t Read();
    
    // последний прочитанный
    static uint8_t GetLastCode() { return last_code; }

    // нажата ли Fn
    static uint8_t IsFnPressed() { return FnPressed; }
    static void SetKeyPressed() { KeyPressed = true; }
    static uint8_t IsKeyPressed() { return KeyPressed; }
};

#endif
