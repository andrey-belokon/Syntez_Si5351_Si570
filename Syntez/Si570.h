#ifndef SI570_H
#define SI570_H

class Si570
{
public:
  Si570() {}
  
  void setup(uint32_t calibration_frequency);

  // in Hz
  bool set_freq(uint32_t newfreq);

  void out_calibrate_freq();

private:
  uint8_t dco_reg[6];
  uint32_t f_center;
  uint32_t frequency;
  uint16_t hs, n1;
  uint32_t freq_xtal;
  uint64_t fdco;
  uint64_t rfreq;
  uint32_t max_delta;

  uint8_t i2c_read_reg(uint8_t reg_address);
  int i2c_read_reg(uint8_t reg_address, uint8_t *output, uint8_t length);

  void i2c_write_reg(uint8_t reg_address, uint8_t data);
  void i2c_write_reg(uint8_t reg_address, uint8_t *data, uint8_t length);

  bool read_si570();
  void write_si570();
  void qwrite_si570();

  uint8_t getHSDIV();
  uint8_t getN1();
  uint64_t getRFREQ();

  void setRFREQ(uint32_t fnew);
  bool findDivisors(uint32_t f);
};

#endif

