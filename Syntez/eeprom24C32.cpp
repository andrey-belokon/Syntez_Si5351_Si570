// based on code https://github.com/jlesech/Eeprom24C04_08_16
// rewriten by Andrii Bilokon for fast i2c
/**************************************************************************//**
 * \brief EEPROM 24C04 / 24C16 library for Arduino
 * \author Copyright (C) 2012  Julien Le Sech - www.idreammicro.com
 * \version 1.0
 * \date 20120218
 *
 * This file is part of the EEPROM 24C04 / 24C16 library for Arduino.
 *
 * This library is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/
 ******************************************************************************/

#include "Eeprom24C32.h"
#include "i2c.h"

#define EEPROM__PAGE_SIZE         32

/**************************************************************************//**
 * \def EEPROM__RD_BUFFER_SIZE
 * \brief Size of input TWI buffer.
 * This size is equal to BUFFER_LENGTH defined in Wire library (32 bytes).
 ******************************************************************************/
#define BUFFER_LENGTH             128
#define EEPROM__RD_BUFFER_SIZE    BUFFER_LENGTH

void Eeprom24C32::setup()
{
    i2c_init();
}

void Eeprom24C32::writeByte
(
    word    address,
    byte    data
){
    i2c_begin_write(m_deviceAddress);
    i2c_write(address >> 8);
    i2c_write(address & 0xFF);
    i2c_write(data);
    i2c_end();
}

void Eeprom24C32::writeBytes
(
    word    address,
    word    length,
    byte*   p_data
){
/*    while (length-- > 0) {
      writeByte(address++, *p_data++);
    }
    return; */

    // Write first page if not aligned.
    byte notAlignedLength = 0;
    byte pageOffset = address % EEPROM__PAGE_SIZE;
    if (pageOffset > 0)
    {
        notAlignedLength = EEPROM__PAGE_SIZE - pageOffset;
        if (length < notAlignedLength)
        {
            notAlignedLength = length;
        }
        writeBuffer(address, notAlignedLength, p_data);
        length -= notAlignedLength;
    }

    if (length > 0)
    {
        address += notAlignedLength;
        p_data += notAlignedLength;

        // Write complete and aligned pages.
        byte pageCount = length / EEPROM__PAGE_SIZE;
        for (byte i = 0; i < pageCount; i++)
        {
            writeBuffer(address, EEPROM__PAGE_SIZE, p_data);
            address += EEPROM__PAGE_SIZE;
            p_data += EEPROM__PAGE_SIZE;
            length -= EEPROM__PAGE_SIZE;
        }

        if (length > 0)
        {
            // Write remaining uncomplete page.
            writeBuffer(address, length, p_data);
        }
    }
}

byte Eeprom24C32::readByte
(
    word address
){
    i2c_begin_write(m_deviceAddress);
    i2c_write(address >> 8);
    i2c_write(address & 0xFF);
    i2c_begin_read(m_deviceAddress);
    byte data = i2c_read();
    i2c_end();
    return data;
}

void Eeprom24C32::readBytes
(
    word    address,
    word    length,
    byte*   p_data
){
/*    while (length-- > 0) {
      *p_data++ = readByte(address++);
    }
    return; */
    
    byte bufferCount = length / EEPROM__RD_BUFFER_SIZE;
    for (byte i = 0; i < bufferCount; i++)
    {
        word offset = i * EEPROM__RD_BUFFER_SIZE;
        readBuffer(address + offset, EEPROM__RD_BUFFER_SIZE, p_data + offset);
    }

    byte remainingBytes = length % EEPROM__RD_BUFFER_SIZE;
    word offset = length - remainingBytes;
    readBuffer(address + offset, remainingBytes, p_data + offset);
}

void Eeprom24C32::writeBuffer
(
    word    address,
    byte    length,
    byte*   p_data
){
    i2c_begin_write(m_deviceAddress);
    i2c_write(address >> 8);
    i2c_write(address & 0xFF);
    for (byte i = 0; i < length; i++)
    {
        i2c_write(p_data[i]);
    }
    i2c_end();
    
    // Write cycle time (tWR). See EEPROM memory datasheet for more details.
    delay(10);
}

void Eeprom24C32::readBuffer
(
    word    address,
    byte    length,
    byte*   p_data
){
    i2c_begin_write(m_deviceAddress);
    i2c_write(address >> 8);
    i2c_write(address & 0xFF);
    i2c_begin_read(m_deviceAddress);
    i2c_read(p_data, length);
    i2c_end();
}
