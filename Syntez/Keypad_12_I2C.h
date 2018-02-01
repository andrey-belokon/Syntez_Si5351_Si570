#ifndef KEYPAD12I2C_H
#define KEYPAD12I2C_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

/*
 * матричная клавиатура 3х4 на PCF8574
 * строки в пинам 0..2, столбцы к пинам 4..7
 */

class Keypad_12_I2C {
  private:
    uint8_t i2c_addr;
    uint8_t last_code;
    long last_code_tm;
    
    void pcf8574_write(uint8_t data);
    uint8_t pcf8574_byte_read();
    uint8_t read_scan();
  public:
	  Keypad_12_I2C(uint8_t _i2c_addr): i2c_addr(_i2c_addr), last_code(-1), last_code_tm(0) {}
    
    void setup();
    
    // возвращает код нажатой клавиши
    // cmdNone если ничего не нажато. подавляет дребезг
    uint8_t Read();
    // последний прочитанный
    uint8_t GetLastCode() { return last_code; }
    // not used
    uint8_t IsFnPressed() { return false; }
    void SetKeyPressed() { }
};

#endif
