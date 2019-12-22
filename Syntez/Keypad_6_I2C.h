#ifndef KEYPAD6I2C_H
#define KEYPAD6I2C_H

#include <Arduino.h>

/*
 * 6-buttons keypad on PCF8574
 * button connected to P0..P5
 */

class Keypad_6_I2C {
  private:
    uint8_t i2c_addr;
    uint8_t last_code;
    uint8_t FnPressed,KeyPressed;
    long last_code_tm;
    long fn_press_tm;
    
    void pcf8574_write(uint8_t data);
    uint8_t pcf8574_byte_read();
  public:
	  Keypad_6_I2C(uint8_t _i2c_addr): i2c_addr(_i2c_addr), last_code(-1), FnPressed(0), KeyPressed(0), last_code_tm(0), fn_press_tm(0) {}
    
    void setup();
    
    // возвращает код нажатой клавиши
    // cmdNone если ничего не нажато. подавляет дребезг
    uint8_t Read();
    // последний прочитанный
    uint8_t GetLastCode() { return last_code; }
    // нажата ли Fn
    uint8_t IsFnPressed() { return FnPressed; }
    void SetKeyPressed() { KeyPressed = true; }
    uint8_t IsKeyPressed() { return KeyPressed; }
};

#endif
