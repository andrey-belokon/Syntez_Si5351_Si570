#ifndef CONFIG_H
#define CONFIG_H

#define LSB 0
#define USB 1

// число диапазонов
#define BAND_COUNT  9

extern const struct _Bands {
  uint8_t   mc;
  long  start, startSSB, end;
  uint8_t sideband;
} Bands[];

#define DEFINED_BANDS \
  {160,  1810000L,  1840000L,  2000000L, LSB}, \
  {80,   3500000L,  3600000L,  3800000L, LSB}, \
  {40,   7000000L,  7045000L,  7200000L, LSB}, \
  {30,  10100000L,        0,  10150000L, USB}, \
  {20,  14000000L, 14100000L, 14350000L, USB}, \
  {17,  18068000L, 18110000L, 18168000L, USB}, \
  {15,  21000000L, 21150000L, 21450000L, USB}, \
  {12,  24890000L, 24930000L, 25140000L, USB}, \
  {10,  28000000L, 28200000L, 29700000L, USB}

// для режима general coverage
#define FREQ_MIN  1000000L
#define FREQ_MAX 30000000L

// список комманд трансивера. порядок критичен и соответствует обходу физического расположения клавиш слева направо, снизу вверх (7-btn keypad)
enum {
  cmdNone = 0,
  
  // без нажатия Fn  
  cmdBandUp,   // переключение диапазонов или частоты
  cmdBandDown,
  cmdLock,     // Lock freq
  cmdVFOSel,   // VFO A/B
  cmdAttPre,   // переключает по кругу аттенюатор/увч
  cmdVFOEQ,    // VFO A=B

  // с нажатой Fn  
  cmdRIT,      // RIT
  cmdZero,     // устанавливает частоту точно по еденицам кГц. 3623145->3623000
  cmdUSBLSB,   // выбор боковой USB/LSB
  cmdSplit,    // Split on/off
  cmdQRP,      // режим уменьшенной выходной мощности
  cmdHam,      // режим Ham band/General coverage. в режиме Ham кнопки cmdBandUp/Down переключают диапазоны
               // в режиме General coverage - изменяют частоту на +/-1MHz
  
  cmdMenu
};

// конфиг "железа"
#include "config_hw.h"

// настройки диапазонов и схемы генерируемых частот 
#include "config_sw.h"

#endif
