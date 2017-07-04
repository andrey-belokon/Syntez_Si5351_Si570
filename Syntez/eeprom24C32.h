// 24C32 and fast i2c (c) Andrii Bilokon 

#ifndef Eeprom24C32_h
#define Eeprom24C32_h

#include <Arduino.h>

class Eeprom24C32
{
    public:
        Eeprom24C32(uint8_t deviceAddress): m_deviceAddress(deviceAddress) {}

        void setup();

        void writeBytes
        (
            uint16_t    address,
            uint16_t    length,
            uint8_t*   p_data
        );
        
        void readBytes
        (
            uint16_t    address,
            uint16_t    length,
            uint8_t*   p_buffer
        );
        
    private:

        uint8_t m_deviceAddress;

        void writePage
        (
            uint16_t    address,
            uint8_t    length,
            uint8_t*   p_data
       );
};

#endif // Eeprom24C32_h

