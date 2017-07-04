/*
 * Si570 Library for Arduino
 *
 * MIT License
 *
 * Modify by Andrey Belokon UR5FFR
 * Copyright Jeff Whitlatch - ko7m - 2014
 * Based on previous work by Thomas Sarlandie which was
 * based on previous work by Ashar Farhan
 */

#include <Arduino.h>
#include "Si570.h"
#include "i2c.h"

#define SI570_I2C_ADDR  0x55

void Si570::setup(uint32_t calibration_frequency)
{
  i2c_init();
  // We are about the reset the Si570, so set the current and center frequency to the calibration frequency.
  f_center = frequency = calibration_frequency;
  max_delta = ((uint64_t) f_center * 10035LL / 10000LL) - f_center;
  // Force Si570 to reset to initial freq
  i2c_write_reg(135,0x01);
  delay(20);
  read_si570();
  // Successfully initialized Si570
  freq_xtal = (unsigned long) ((uint64_t) calibration_frequency * getHSDIV() * getN1() * (1L << 28) / getRFREQ());
/*
  Serial.println(getHSDIV(),HEX);
  Serial.println(getN1(),HEX);
  Serial.println((uint16_t)(getRFREQ() >> 16),HEX);
  Serial.println((uint16_t)(getRFREQ() & 0xFFFF),HEX);
*/  
}

void Si570::out_calibrate_freq()
{
  
}

// Return the 8 bit HSDIV value from register 7
uint8_t Si570::getHSDIV()
{
  uint8_t hs_reg_value = dco_reg[0] >> 5;
  return 4 + hs_reg_value;
}

// Compute and return the 8 bit N1 value from registers 7 and 8
uint8_t Si570::getN1()
{
  uint8_t n_reg_value = ((dco_reg[0] & 0x1F) << 2) + (dco_reg[1] >> 6);
  return n_reg_value + 1;
}

// Return 38 bit RFREQ value in a 64 bit integer
uint64_t Si570::getRFREQ()
{
  fdco  = (uint64_t)(dco_reg[1] & 0x3F) << 32;
  fdco |= (uint64_t) dco_reg[2] << 24;
  fdco |= (uint64_t) dco_reg[3] << 16;
  fdco |= (uint64_t) dco_reg[4] << 8;
  fdco |= (uint64_t) dco_reg[5];

  return fdco;
}

// Write a byte to I2C device
void Si570::i2c_write_reg(uint8_t reg_address, uint8_t data)
{
  i2c_begin_write(SI570_I2C_ADDR);
  i2c_write(reg_address);
  i2c_write(data);
  i2c_end();
}

// Write length bytes to I2C device.
void Si570::i2c_write_reg(uint8_t reg_address, uint8_t *data, uint8_t length)
{
  i2c_begin_write(SI570_I2C_ADDR);
  i2c_write(reg_address);
  while (length-- > 0)
    i2c_write(*data++);
  i2c_end();
  return length;
}

// Read a one byte register from the I2C device
uint8_t Si570::i2c_read_reg(uint8_t reg_address) 
{
  i2c_begin_write(SI570_I2C_ADDR);
  i2c_write(reg_address);
  i2c_begin_read(SI570_I2C_ADDR);
  uint8_t data = i2c_read();
  i2c_end();
  return data;
}

// Read multiple bytes fromt he I2C device
int Si570::i2c_read_reg(uint8_t reg_address, uint8_t *output, uint8_t length) 
{
  i2c_begin_write(SI570_I2C_ADDR);
  i2c_write(reg_address);
  i2c_begin_read(SI570_I2C_ADDR);
  i2c_read(output,length);
  i2c_end();
  return length;
}

// Read the Si570 chip and populate dco_reg values
bool Si570::read_si570()
{
  i2c_read_reg(7, &(dco_reg[0]), 6);
  return true;
}

// Write dco_reg values to the Si570
void Si570::write_si570()
{
  int idco;

  // Freeze DCO
  idco = i2c_read_reg(137);
  i2c_write_reg(137, idco | 0x10 );

  i2c_write_reg(7, &dco_reg[0], 6);

  // Unfreeze DCO
  i2c_write_reg(137, idco & 0xEF);

  // Set new freq
  i2c_write_reg(135,0x40);
}

// In the case of a frequency change < 3500 ppm, only RFREQ must change
void Si570::qwrite_si570()
{
  int idco;

  // Freeze the M Control Word to prevent interim frequency changes when writing RFREQ registers.
  idco = i2c_read_reg(135);
  i2c_write_reg(135, idco | 0x20);

  // Write RFREQ registers
  i2c_write_reg(7, &dco_reg[0], 6);

  // Unfreeze the M Control Word
  i2c_write_reg(135, idco &  0xdf);
}

#define fDCOMinkHz 4850000	// Minimum DCO frequency in kHz
#define fDCOMaxkHz 5670000  // Maximum DCO frequency in KHz\

// Locate an appropriate set of divisors (HSDiv and N1) give a desired output frequency
bool Si570::findDivisors(uint32_t fout)
{
  const uint16_t HS_DIV[] = {11, 9, 7, 6, 5, 4};
  uint32_t fout_kHz = fout / 1000;

  // Floor of the division
  uint16_t maxDivider = fDCOMaxkHz / fout_kHz;
 
  // Ceiling of the division
  n1 = 1 + ((fDCOMinkHz - 1) / fout_kHz / 11);

  if (n1 < 1 || n1 > 128)
    return false;

  while (n1 <= 128) 
  {
    if (0 == n1 % 2 || 1 == n1)
    {
      // Try each divisor from largest to smallest order to minimize power
      for (int i = 0; i < 6 ; ++i) 
      {
        hs = HS_DIV[i];
        if (hs * n1 <= maxDivider) 
          return true;
      }
    }
    n1++;
  }
  return false;
}

// Set RFREQ register (38 bits)
void Si570::setRFREQ(uint32_t fnew)
{
  // Calculate new DCO frequency
  fdco = (uint64_t) fnew * hs * n1;

  // Calculate the new RFREQ value
  rfreq = (fdco << 28) / freq_xtal;

  // Round the result
  rfreq = rfreq + ((rfreq & 1<<(28-1))<<1);

  // Set up the RFREQ register values
  dco_reg[5] = rfreq & 0xff;
  dco_reg[4] = rfreq >> 8 & 0xff;
  dco_reg[3] = rfreq >> 16 & 0xff;
  dco_reg[2]  = rfreq >> 24 & 0xff;
  dco_reg[1]  = rfreq >> 32 & 0x3f;

  // set up HS and N1 in registers 7 and 8
  dco_reg[0]  = (hs - 4) << 5;
  dco_reg[0]  = dco_reg[0] | ((n1 - 1) >> 2);
  dco_reg[1] |= ((n1-1) & 0x3) << 6;
}

// Set the Si570 frequency
bool Si570::set_freq(uint32_t newfreq) 
{
  // If the current frequency has not changed, we are done
  if (frequency != newfreq) {
    // Check how far we have moved the frequency (without using abs() function)
    uint32_t delta_freq = newfreq < f_center ? f_center - newfreq : newfreq - f_center;
  
    // If the jump is small enough, we don't have to fiddle with the dividers
    if (delta_freq < max_delta) {
      setRFREQ(newfreq);
      frequency = newfreq;
      qwrite_si570();
    } else {
      // otherwise it is a big jump and we need a new set of divisors and reset center frequency
      if (!findDivisors(newfreq)) return false;
      setRFREQ(newfreq);
      f_center = frequency = newfreq;
  	  // Calculate the new 3500 ppm delta
  	  max_delta = ((uint64_t) f_center * 10035LL / 10000LL) - f_center;
      write_si570();
    }
  }
  return true;
}
