#ifndef CONFIG_H
#define CONFIG_H

#define LSB 0
#define USB 1

// predefined modes - index in Modes[] (see config_sw.h)
#define MODE_SSB  0
#define MODE_CW   1

extern const struct _Bands {
  long  start, end;
  uint8_t mode;
  uint8_t sideband;
} Bands[];

// для каждого бенда задаем начало/конец, моду и полосу.

#define DEFINED_BANDS \
  { 1810000L,  2000000L, MODE_SSB, LSB}, \
  { 3500000L,  3800000L, MODE_SSB, LSB}, \
  { 7000000L,  7200000L, MODE_SSB, LSB}, \
  {10100000L, 10150000L, MODE_SSB, USB}, \
  {14000000L, 14350000L, MODE_SSB, USB}, \
  {18068000L, 18168000L, MODE_SSB, USB}, \
  {21000000L, 21450000L, MODE_SSB, USB}, \
  {24890000L, 25140000L, MODE_SSB, USB}, \
  {28000000L, 29700000L, MODE_SSB, USB}

// для режима general coverage
#define FREQ_MIN  1000000L
#define FREQ_MAX 30000000L
// закоментировать для отключения режима непрерывного перекрытия
#define GENERAL_COVERAGE_ENABLED

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
  cmdMenu,
  cmdMode,     // смена моды CW/SSB/AM
  cmdTune      // режим настройки - TX+QRP
};

// конфиг "железа"
#include "config_hw.h"

// настройки диапазонов и схемы генерируемых частот 
#include "config_sw.h"

#endif
