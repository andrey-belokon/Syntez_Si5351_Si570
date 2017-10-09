#include <inttypes.h>
#include "si5351a.h"
#include "i2c.h"

#define SI_CLK0_CONTROL 16      // Register definitions
#define SI_CLK1_CONTROL 17
#define SI_CLK2_CONTROL 18
#define SI_SYNTH_PLL_A  26
#define SI_SYNTH_PLL_B  34
#define SI_SYNTH_MS_0   42
#define SI_SYNTH_MS_1   50
#define SI_SYNTH_MS_2   58
#define SI_PLL_RESET    177

#define SI_PLL_RESET_A   0x20
#define SI_PLL_RESET_B   0x80

#define SI_CLK0_PHASE  165
#define SI_CLK1_PHASE  166
#define SI_CLK2_PHASE  167

#define SI_R_DIV_1    0b00000000      // R-division ratio definitions
#define SI_R_DIV_2    0b00010000
#define SI_R_DIV_4    0b00100000
#define SI_R_DIV_8    0b00110000
#define SI_R_DIV_16   0b01000000
#define SI_R_DIV_32   0b01010000
#define SI_R_DIV_64   0b01100000
#define SI_R_DIV_128  0b01110000

#define R_DIV(x) ((x) << 4)

#define SI_CLK_SRC_PLL_A  0b00000000
#define SI_CLK_SRC_PLL_B  0b00100000

#define SI5351_I2C_ADDR 0x60

#define FRAC_DENOM 0xFFFFF

void si5351_write_reg(uint8_t reg, uint8_t data)
{
  i2c_begin_write(SI5351_I2C_ADDR);
  i2c_write(reg);
  i2c_write(data);
  i2c_end();
}

void si5351_write_regs(uint8_t synth, uint32_t P1, uint32_t P2, uint32_t P3, uint8_t rDiv)
{
  i2c_begin_write(SI5351_I2C_ADDR);
  i2c_write(synth);
  i2c_write(((uint8_t*)&P3)[1]);
  i2c_write((uint8_t)P3);
  i2c_write((((uint8_t*)&P1)[2] & 0x3) | rDiv);
  i2c_write(((uint8_t*)&P1)[1]);
  i2c_write((uint8_t)P1);
  i2c_write(((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  i2c_write(((uint8_t*)&P2)[1]);
  i2c_write((uint8_t)P2);
  i2c_end();
}

// Set up MultiSynth with mult, num and denom
// mult is 15..90
// num is 0..1,048,575 (0xFFFFF)
// denom is 0..1,048,575 (0xFFFFF)
//
void si5351_setup_msynth(uint8_t synth, uint8_t a, uint32_t b, uint32_t c, uint8_t rDiv)
{
  uint32_t t = 128*b / c;
  si5351_write_regs(
    synth,
    (uint32_t)(128 * (uint32_t)(a) + t - 512),
    (uint32_t)(128 * b - c * t),
    c,
    rDiv
  );
}

//
// Set up MultiSynth with integer divider and R divider
// R divider is the bit value which is OR'ed onto the appropriate register, it is a #define in si5351a.h
//
void si5351_setup_msynth_int(uint8_t synth, uint32_t divider, uint8_t rDiv)
{
  // P2 = 0, P3 = 1 forces an integer value for the divider
  si5351_write_regs(
    synth,
    128 * divider - 512,
    0,
    1,
    rDiv
  );
}

void Si5351::setup(uint8_t _power0, uint8_t _power1, uint8_t _power2)
{
  i2c_init();
  power0 = _power0 & 0x3;
  power1 = _power1 & 0x3;
  power2 = _power2 & 0x3;
  si5351_write_reg(SI_CLK0_CONTROL, 0x80);
  si5351_write_reg(SI_CLK1_CONTROL, 0x80);
  si5351_write_reg(SI_CLK2_CONTROL, 0x80);
}

void Si5351::set_xtal_freq(uint32_t freq, uint8_t reset_pll)
{
  uint8_t need_reset_pll;
  xtal_freq = freq*10;
  update_freq0(&need_reset_pll);
  update_freq12(1,&need_reset_pll);
  if (reset_pll) si5351_write_reg(SI_PLL_RESET, 0xA0);
}

uint8_t Si5351::set_freq(uint32_t f0, uint32_t f1, uint32_t f2)
{
  uint8_t need_reset_pll = 0;
  uint8_t freq1_changed = f1 != freq1;
  if (f0 != freq0) {
    freq0 = f0;
    update_freq0(&need_reset_pll);
  }
  if (freq1_changed || f2 != freq2) {
    freq1 = f1;
    freq2 = f2;
    update_freq12(freq1_changed,&need_reset_pll);
  }
  if (need_reset_pll) 
    si5351_write_reg(SI_PLL_RESET, need_reset_pll);
  return need_reset_pll;
}

void Si5351::disable_out(uint8_t clk_num)
{
  switch (clk_num) {
    case 0: 
      si5351_write_reg(SI_CLK0_CONTROL, 0x80);
      freq0_div = 0;
      break;
    case 1: 
      si5351_write_reg(SI_CLK1_CONTROL, 0x80);
      freq1_div = 0;
      break;
    case 2: 
      si5351_write_reg(SI_CLK2_CONTROL, 0x80);
      freq2_div = 0;
      break;
  }
}

uint8_t Si5351::is_freq_ok(uint8_t clk_num)
{
  switch (clk_num) {
    case 0: 
      return freq0_div != 0;
      break;
    case 1: 
      return freq1_div != 0;
      break;
    case 2: 
      return freq2_div != 0;
      break;
  }
}

void Si5351::out_calibrate_freq()
{
  si5351_write_reg(SI_CLK0_CONTROL, power0);
  si5351_write_reg(SI_CLK1_CONTROL, power1);
  si5351_write_reg(SI_CLK2_CONTROL, power2);
  si5351_write_reg(SI_SYNTH_MS_0+2,0);
  si5351_write_reg(SI_SYNTH_MS_1+2,0);
  si5351_write_reg(SI_SYNTH_MS_2+2,0);
  si5351_write_reg(187, 0xD0);
  freq0=freq1=freq2=xtal_freq;
}

void Si5351::update_freq0(uint8_t* need_reset_pll)
{
  uint64_t pll_freq;
  uint8_t mult;
  uint32_t num;
  uint32_t divider;
  uint8_t rdiv = 0;

  if (freq0 == 0) {
    disable_out(0);
    return;
  }
  
  divider = 900000000 / freq0;
  if (divider < 6) {
    disable_out(0);
    return;
  }
  while (divider > 300) {
    rdiv++;
    divider >>= 1;
  }
  if (rdiv == 0) divider &= 0xFFFFFFFE;

  pll_freq = divider * freq0 * (1 << rdiv);

  mult = pll_freq*10 / xtal_freq;
  num = (pll_freq - (uint64_t)mult*xtal_freq/10)*FRAC_DENOM*10/xtal_freq;

  si5351_setup_msynth(SI_SYNTH_PLL_A, mult, num, FRAC_DENOM, 0);

  if (divider != freq0_div || rdiv != freq0_rdiv) {
    si5351_setup_msynth_int(SI_SYNTH_MS_0, divider, R_DIV(rdiv));
    si5351_write_reg(SI_CLK0_CONTROL, 0x4C | power0 | SI_CLK_SRC_PLL_A);
    freq0_div = divider;
    freq0_rdiv = rdiv;
    *need_reset_pll |= SI_PLL_RESET_A;
  }
}

void Si5351::update_freq12(uint8_t freq1_changed, uint8_t* need_reset_pll)
{
  uint64_t pll_freq;
  uint8_t mult;
  uint32_t num;
  uint32_t ff;
  uint32_t divider;
  uint8_t rdiv = 0;

  if (freq1 == 0) {
    disable_out(1);
  }
  
  if (freq2 == 0) {
    disable_out(2);
  }

  if (freq1) {
    if (freq1_changed) {
      divider = 900000000 / freq1;
      if (divider < 6) {
        disable_out(1);
        return;
      }
      while (divider > 300) {
        rdiv++;
        divider >>= 1;
      }
      if (rdiv == 0) divider &= 0xFFFFFFFE;
    
      pll_freq = divider * freq1 * (1 << rdiv);
    
      mult = pll_freq*10 / xtal_freq;
      num = (pll_freq - (uint64_t)mult*xtal_freq/10)*FRAC_DENOM*10/xtal_freq;
    
      si5351_setup_msynth(SI_SYNTH_PLL_B, mult, num, FRAC_DENOM, 0);
      if (divider != freq1_div || rdiv != freq1_rdiv) {
        si5351_setup_msynth_int(SI_SYNTH_MS_1, divider, R_DIV(rdiv));
        si5351_write_reg(SI_CLK1_CONTROL, 0x4C | power1 | SI_CLK_SRC_PLL_B);
        if (freq2) {
          si5351_write_reg(SI_CLK2_CONTROL, 0x4C | power2 | SI_CLK_SRC_PLL_B);
        }
        freq1_div = divider;
        freq1_rdiv = rdiv;
        *need_reset_pll |= SI_PLL_RESET_B;
      }
      freq_pll_b = pll_freq;
    }

    if (freq2) {
      // CLK2 --> PLL_B with fractional or integer multisynth 
      divider = freq_pll_b / freq2;
      if (divider < 6) {
        disable_out(2);
        return;
      }
      rdiv = 0;
      ff = freq2;
      while (divider > 300) {
        rdiv++;
        ff <<= 1;
        divider >>= 1;
      }
      num = (uint64_t)(freq_pll_b % ff) * FRAC_DENOM / ff;
  
      si5351_setup_msynth(SI_SYNTH_MS_2,divider, num, (num?FRAC_DENOM:1), R_DIV(rdiv));
      freq2_div = 1; // non zero for correct enable/disable CLK2
    }
  } else if (freq2) {
    // PLL_B --> CLK2, multisynth integer
    divider = 900000000 / freq2;
    if (divider < 6) {
      disable_out(2);
      return;
    }
    while (divider > 300) {
      rdiv++;
      divider >>= 1;
    }
    if (rdiv == 0) divider &= 0xFFFFFFFE;
  
    pll_freq = divider * freq2 * (1 << rdiv);
  
    mult = pll_freq*10 / xtal_freq;
    num = (pll_freq - (uint64_t)mult*xtal_freq/10)*FRAC_DENOM*10/xtal_freq;
  
    si5351_setup_msynth(SI_SYNTH_PLL_B, mult, num, FRAC_DENOM, 0);
  
    if (divider != freq2_div || rdiv != freq2_rdiv) {
      si5351_setup_msynth_int(SI_SYNTH_MS_2, divider, R_DIV(rdiv));
      si5351_write_reg(SI_CLK2_CONTROL, 0x4C | power2 | SI_CLK_SRC_PLL_B);
      freq2_div = divider;
      freq2_rdiv = rdiv;
      *need_reset_pll |= SI_PLL_RESET_B;
    }
  }
}

void Si5351::update_freq01_quad(uint8_t* need_reset_pll)
{
  uint64_t pll_freq;
  uint8_t mult;
  uint32_t num;
  uint32_t divider;

  if (freq0 == 0) {
    disable_out(0);
    disable_out(1);
    return;
  }
  
  if (freq0 >= 7000000) {
    divider = (900000000 / freq0);
  } else if (freq0 >= 4000000) {
    divider = (600000000 / freq0);
  } else if (freq0 >= 2000000) {
    // VFO run on freq less than 600MHz. possible unstable
    // comment this for disable operation below 600MHz VFO (4MHz on out)
    divider = 0x7F;
  } else {
    divider = 0; // disable out on invalid freq
  }
  if (divider < 6 || divider > 0x7F) {
    disable_out(0);
    disable_out(1);
    return;
  }

  pll_freq = divider * freq0;

  mult = pll_freq*10 / xtal_freq;
  num = (pll_freq - (uint64_t)mult*xtal_freq/10)*FRAC_DENOM*10/xtal_freq;

  si5351_setup_msynth(SI_SYNTH_PLL_A, mult, num, FRAC_DENOM, 0);

  if (divider != freq0_div) {
    si5351_setup_msynth_int(SI_SYNTH_MS_0, divider, 0);
    si5351_write_reg(SI_CLK0_CONTROL, 0x4C | power0 | SI_CLK_SRC_PLL_A);
    si5351_write_reg(SI_CLK0_PHASE, 0);
    si5351_setup_msynth_int(SI_SYNTH_MS_1, divider, 0);
    si5351_write_reg(SI_CLK1_CONTROL, 0x4C | power0 | SI_CLK_SRC_PLL_A);
    si5351_write_reg(SI_CLK1_PHASE, divider & 0x7F);
    freq0_div = freq1_div = divider;
    *need_reset_pll |= SI_PLL_RESET_A;
  }
}

void Si5351::update_freq2(uint8_t* need_reset_pll)
{
  uint64_t pll_freq;
  uint8_t mult;
  uint32_t num;
  uint32_t ff;
  uint32_t divider;
  uint8_t rdiv = 0;

  if (freq2 == 0) {
    disable_out(2);
    return;
  }

  // PLL_B --> CLK2, multisynth integer
  divider = 900000000 / freq2;
  if (divider < 6) {
    disable_out(2);
    return;
  }
  while (divider > 300) {
    rdiv++;
    divider >>= 1;
  }
  if (rdiv == 0) divider &= 0xFFFFFFFE;

  pll_freq = divider * freq2 * (1 << rdiv);

  mult = pll_freq*10 / xtal_freq;
  num = (pll_freq - (uint64_t)mult*xtal_freq/10)*FRAC_DENOM*10/xtal_freq;

  si5351_setup_msynth(SI_SYNTH_PLL_B, mult, num, FRAC_DENOM, 0);

  if (divider != freq2_div || rdiv != freq2_rdiv) {
    si5351_setup_msynth_int(SI_SYNTH_MS_2, divider, R_DIV(rdiv));
    si5351_write_reg(SI_CLK2_CONTROL, 0x4C | power2 | SI_CLK_SRC_PLL_B);
    freq2_div = divider;
    freq2_rdiv = rdiv;
    *need_reset_pll |= SI_PLL_RESET_B;
  }
}

uint8_t Si5351::set_freq_quadrature(uint32_t f01, uint32_t f2)
{
  uint8_t need_reset_pll = 0;
  if (f01 != freq0) {
    freq0 = f01;
    update_freq01_quad(&need_reset_pll);
  }
  if (f2 != freq2) {
    freq2 = f2;
    update_freq2(&need_reset_pll);
  }
  if (need_reset_pll) 
    si5351_write_reg(SI_PLL_RESET, need_reset_pll);
  return need_reset_pll;
}

