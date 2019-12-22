// 24C32 and fast i2c (c) Andrii Bilokon 

#include "Eeprom24C32.h"
#include "i2c.h"

#define EEPROM__PAGE_SIZE         32

void Eeprom24C32::setup()
{

}

bool Eeprom24C32::found()
{
  static uint8_t _pooled = false;
  static uint8_t _found = false;
  if (!_pooled) _found = i2c_device_found(m_deviceAddress);
  return _found;
}

void Eeprom24C32::writeBytes
(
    uint16_t    address,
    uint16_t    length,
    uint8_t*   p_data
){
    // Write first page if not aligned.
    uint8_t notAlignedLength = 0;
    uint8_t pageOffset = address % EEPROM__PAGE_SIZE;
    if (pageOffset > 0)
    {
        notAlignedLength = EEPROM__PAGE_SIZE - pageOffset;
        if (length < notAlignedLength)
        {
            notAlignedLength = length;
        }
        writePage(address, notAlignedLength, p_data);
        length -= notAlignedLength;
    }

    if (length > 0)
    {
        address += notAlignedLength;
        p_data += notAlignedLength;

        // Write complete and aligned pages.
        uint8_t pageCount = length / EEPROM__PAGE_SIZE;
        for (uint8_t i = 0; i < pageCount; i++)
        {
            writePage(address, EEPROM__PAGE_SIZE, p_data);
            address += EEPROM__PAGE_SIZE;
            p_data += EEPROM__PAGE_SIZE;
            length -= EEPROM__PAGE_SIZE;
        }

        if (length > 0)
        {
            // Write remaining uncomplete page.
            writePage(address, length, p_data);
        }
    }
}

void Eeprom24C32::readBytes
(
    uint16_t    address,
    uint16_t    length,
    uint8_t*   p_data
){   
  i2c_begin_write(m_deviceAddress);
  i2c_write(address >> 8);
  i2c_write(address & 0xFF);
  i2c_begin_read(m_deviceAddress);
  i2c_read_long(p_data, length);
  i2c_end();
}

void Eeprom24C32::writePage
(
    uint16_t    address,
    uint8_t    length,
    uint8_t*   p_data
){
    i2c_begin_write(m_deviceAddress);
    i2c_write(address >> 8);
    i2c_write(address & 0xFF);
    for (uint8_t i = 0; i < length; i++)
    {
        i2c_write(p_data[i]);
    }
    i2c_end();
    
    // Write cycle time (tWR). See EEPROM memory datasheet for more details.
    delay(10);
}
