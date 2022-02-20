#ifndef CONFIG_HW_H
#define CONFIG_HW_H

// раскоментировать используемый дисплей (только один!). закоментировать все если нет дисплея
#define DISPLAY_ST7735
//#define DISPLAY_ILI9341
//#define DISPLAY_1602
//#define DISPLAY_OLED128x32     // OLED 0.91" 128x32
//#define DISPLAY_OLED128x64     // OLED 0.96" 128x64
//#define DISPLAY_OLED_SH1106_128x64     // OLED 1.3" 128x64 (132x64)

// ST7735 has several variations, set your version based on this list (using the color of the "tab" on the screen cover).
// NOTE: The tab colors refer to Adafruit versions, other suppliers may vary (you may have to experiment to find the right one).
// при неправильном указании варианта некоректно отображаются цвета
enum {
  ST7735_INITB      = 0,        // 1.8" (128x160) ST7735B chipset (only one type)
  ST7735_INITR_GREENTAB   = 1,        // 1.8" (128x160) ST7735R chipset with green tab (same as ST7735_INITR_18GREENTAB)
  ST7735_INITR_REDTAB   = 2,        // 1.8" (128x160) ST7735R chipset with red tab (same as ST7735_INITR_18REDTAB)
  ST7735_INITR_BLACKTAB   = 3,        // 1.8" (128x160) ST7735S chipset with black tab (same as ST7735_INITR_18BLACKTAB)
  ST7735_INITR_144GREENTAB    = 4,        // 1.4" (128x128) ST7735R chipset with green tab
  ST7735_INITR_18GREENTAB   = ST7735_INITR_GREENTAB,  // 1.8" (128x160) ST7735R chipset with green tab
  ST7735_INITR_18REDTAB   = ST7735_INITR_REDTAB,    // 1.8" (128x160) ST7735R chipset with red tab
  ST7735_INITR_18BLACKTAB   = ST7735_INITR_BLACKTAB,  // 1.8" (128x160) ST7735S chipset with black tab
};

#define ST7735_CHIPSET ST7735_INITR_BLACKTAB // <= Set ST7735 LCD chipset/variation here

// значения 0/1 определяют ориентацию вывода на TFT
// для двухплатной версии установить в 0 при 10ти пиновом модуле ST7735, и в 1 при 8ми пиновом
#define TFT_ORIENTATION   1

// I2C адрес 1602 LCD
#define I2C_ADR_DISPLAY_1602  0x27

// I2C адрес для OLED
#define I2C_ADD_DISPLAY_OLED  0x3C

// раскоментировать используемую клавиатуру (только одну!). закоментировать все если нет клавиатуры
#define KEYPAD_6          0x3E
//#define KEYPAD_7          0x3E
//#define KEYPAD_12         0x26

// управление RIT потенциометром 0..5в на аналоговый вход. при работе с двумя Si5351 A7 используется для их переключения а RIT управляется валкодером (KEYPAD_6)
#if defined(KEYPAD_7) || defined(KEYPAD_12)
  #define PIN_IN_RIT      A7
#endif
#define RIT_MAX_VALUE   1200      // максимальная расстройка

// раскоментировать установленные чипы
#define VFO_SI5351
#define VFO_SI5351_2
//#define VFO_SI570

#if defined(VFO_SI5351_2) && !defined(VFO_SI5351)
  #error Invalid combination of used SI5351
#endif

#ifdef VFO_SI5351_2
  #ifdef PIN_IN_RIT
    #error KEYPAD_7 cant be used with two SI5351
  #endif
  #define PIN_SELECT_SI5351   A3
#endif

// выбрать в меню калибровку и прописать измеренные частоты на выходах синтезаторов
#define SI5351_CALIBRATION       26000000
#define SI5351_2_CALIBRATION     26000000
#define SI570_CALIBRATION        56319832

// уровень сигнала на выходе Si5351. 0=2mA, 1=4mA, 2=6mA, 3=8mA
#define SI5351_CLK0_DRIVE   3
#define SI5351_CLK1_DRIVE   3
#define SI5351_CLK2_DRIVE   3
#define SI5351_2_CLK0_DRIVE   3
#define SI5351_2_CLK1_DRIVE   3
#define SI5351_2_CLK2_DRIVE   3

// раскоментировать используемый модуль часов Real Time CLock (RTC)
//#define RTC_DS1307
//#define RTC_PCF8563
#define RTC_DS3231

#if defined(RTC_DS1307) || defined(RTC_PCF8563) || defined(RTC_DS3231)
  #define RTC_ENABLE
#endif

/*
  I2C address mapping
  0x25  ! PCF8574/LCD board (alt band control I2C_ADR_BAND_CTRL)
  0x26  ! PCF8574/LCD board (3x4 keypad KEYPAD_12)
  0x27  ! LCD 1602 (DISPLAY_1602)
  0x3B  ! PCF8574 (band control I2C_ADR_BAND_CTRL)
  0x3В  ! PCF8574 (ext control I2C_ADR_EXT_CTRL)
  0x3E  ! PCF8574 (7 btn keypad KEYPAD_7)
  0x50  ! AT24C32 at TinyRTC board or single IC [optional]
  0x55  ! Si570 [optional]
  0x60  ! Si5351 [optional]
  0x68  ! DS1307 at TinyRTC board [optional]
  0x68  ! DS3132 [optional]
*/

// I2C адреса устройств
#ifdef RTC_DS3231
  #define I2C_ADR_EE24C32       0x57
#else
  #define I2C_ADR_EE24C32       0x50
#endif
#define I2C_ADR_BAND_CTRL     0x3B
//#define I2C_ADR_EXT_CTRL      0x3D

// Pin mapping
#define PIN_IN_TX       4
#define PIN_IN_TUNE     5
#define PIN_OUT_TX      6
#define PIN_OUT_QRP     7
#define PIN_OUT_TONE    8
#define PIN_IN_SMETER   A6

#define OUT_TONE_FREQ   1000

#define BANDCTRL_ENABLE
// распиновка I2C расширителя band control
// двоичный дешифратор диапазона - пины 0-3
#define BCPN_BAND_0     0
#define BCPN_BAND_1     1
#define BCPN_BAND_2     2
#define BCPN_BAND_3     3
// 4й - SSB/CW (можно использовать для переключения фильтров/режимов)
// пин отустствует (не разведен) на I2C-LCD адаптерах
#define BCPN_CW         4
#define BCPN_ATT1       4
// 5й пин - ATT, 6й пин - Preamp
#define BCPN_ATT        5
#define BCPN_ATT2       5
#define BCPN_PRE        6
// 7й - LSB - 0, USB - 1 (можно использовать для переключения фильтров/режимов)
#define BCPN_SB         7
#define BCPN_IFSEL      7

// раскоментировать ТОЛЬКО ОДИН требуемый тип энкодера. закоментировать все если нет
#define ENCODER_OPTICAL
//#define ENCODER_MECHANIC
//#define ENCODER_AS5600

// изменение частоты в Гц на один оборот в обычном режиме
#define ENCODER_FREQ_LO_STEP      3000
// изменение частоты в Гц на один оборот в ускоренном режиме
#define ENCODER_FREQ_HI_STEP      12000
// кратность перестройки частоты при нажатой кнопке Fn
#define ENCODER_FN_MULT           10
// порог переключения в ускоренный режим. если частота изменится более
// чем на ENCODER_FREQ_HI_LO_TRASH Гц за секунду то переходим в ускоренный режим
#define ENCODER_FREQ_HI_LO_TRASH  2000

#ifdef ENCODER_AS5600
#define ENCODER_ENABLE
#endif

#ifdef ENCODER_OPTICAL
#ifdef ENCODER_ENABLE
  #error You need select single encoder
#endif
#define ENCODER_ENABLE
// количество импульсов на оборот примененного оптического энкодера
#define ENCODER_PULSE_PER_TURN    360
#endif

#ifdef ENCODER_MECHANIC
#ifdef ENCODER_ENABLE
  #error You need select single encoder
#endif
#define ENCODER_ENABLE
// количество импульсов на оборот примененного механического энкодера
#define ENCODER_PULSE_PER_TURN    20
#endif

#define CAT_ENABLE
#define COM_BAUND_RATE  9600      // скорость обмена COM-порта

// выбрать тип CAT протокола (только один!)
#define CAT_PROTOCOL_KENWOOD_TS480
//#define CAT_PROTOCOL_YAESU_FT817

// интервал опроса S-метра, msec
#define POLL_SMETER     50
// интервал проверки и сохранения текущего состояния синтезатора в EEPROM
#define POLL_EEPROM_STATE 500

#endif
