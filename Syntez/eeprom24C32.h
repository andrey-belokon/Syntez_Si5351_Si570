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

#ifndef Eeprom24C32_h
#define Eeprom24C32_h

#include <Arduino.h>

class Eeprom24C32
{
    public:
        Eeprom24C32(byte deviceAddress): m_deviceAddress(deviceAddress) {}

        void setup();

        void writeByte
        (
            word    address,
            byte    data
        );
        
        void writeBytes
        (
            word    address,
            word    length,
            byte*   p_data
        );
        
        byte readByte
        (
            word    address
        );

        void readBytes
        (
            word    address,
            word    length,
            byte*   p_buffer
        );
        
    private:

        byte m_deviceAddress;

        void writeBuffer
        (
            word    address,
            byte    length,
            byte*   p_data
       );

        void readBuffer
        (
            word    address,
            byte    length,
            byte*   p_data
        );
};

#endif // Eeprom24C32_h

