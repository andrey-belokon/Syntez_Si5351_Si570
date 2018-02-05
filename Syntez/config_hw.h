#ifndef CONFIG_HW_H
#define CONFIG_HW_H

// раскоментировать используемый дисплей (только один!). закоментировать все если нет дисплея
//#define DISPLAY_ST7735
#define DISPLAY_ILI9341
//#define DISPLAY_1602

// раскоментировать используемую клавиатуру (только одну!). закоментировать все если нет клавиатуры
#define KEYPAD_7
//#define KEYPAD_12

// раскоментировать установленные чипы
#define VFO_SI5351
#define VFO_SI570

// выбрать в меню калибровку и прописать измеренные частоты на выходах синтезаторов
#define SI5351_CALIBRATION       25003181
#define SI570_CALIBRATION        56319832

// уровень сигнала на выходе Si5351. 0=2mA, 1=4mA, 2=6mA, 3=8mA
#define SI5351_CLK0_DRIVE   0
#define SI5351_CLK1_DRIVE   0
#define SI5351_CLK2_DRIVE   0

/*
  I2C address mapping
  0x25  ! PCF8574/LCD board (alt band control I2C_ADR_BAND_CTRL)
  0x26  ! PCF8574/LCD board (3x4 keypad KEYPAD_12)
  0x27  ! LCD 1602 (DISPLAY_1602)
  0x3B  ! PCF8574 (band control I2C_ADR_BAND_CTRL)
  0x3E  ! PCF8574 (7 btn keypad KEYPAD_7)
  0x50  ! AT24C32 at TinyRTC board or single IC [optional]
  0x55  ! Si570 [optional]
  0x60  ! Si5351 [optional]
  0x68  ! DS1307 at TinyRTC board [optional]
*/

// I2C адреса устройств
#define I2C_ADR_KEYPAD_7      0x3E
#define I2C_ADR_KEYPAD_12     0x26
#define I2C_ADR_DISPLAY_1602  0x27
#define I2C_ADR_EE24C32       0x50
#define I2C_ADR_BAND_CTRL     0x3B

// Pin mapping
#define PIN_IN_TX       4
#define PIN_IN_TUNE     5
#define PIN_OUT_TX      6
#define PIN_OUT_QRP     7
#define PIN_OUT_TONE    8
#define PIN_IN_SMETER   A6
#define PIN_IN_RIT      A7

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
// 5й пин - ATT, 6й пин - Preamp
#define BCPN_ATT        5
#define BCPN_PRE        6
// 7й - LSB - 0, USB - 1 (можно использовать для переключения фильтров/режимов)
#define BCPN_SB         7

// закоментировать если нет валкодера
#define ENCODER_ENABLE
// количество импульсов на оборот примененного энкодера
#define ENCODER_PULSE_PER_TURN    360
// изменение частоты в Гц на один оборот в обычном режиме
#define ENCODER_FREQ_LO_STEP      3000
// изменение частоты в Гц на один оборот в ускоренном режиме
#define ENCODER_FREQ_HI_STEP      12000
// порог переключения в ускоренный режим. если частота изменится более
// чем на ENCODER_FREQ_HI_LO_TRASH Гц за секунду то переходим в ускоренный режим
#define ENCODER_FREQ_HI_LO_TRASH  8000                   
// кратность перестройки частоты при нажатой кнопке Fn
#define ENCODER_FN_MULT           10

#define CAT_ENABLE
#define COM_BAUND_RATE  9600      // скорость обмена COM-порта

#define RIT_MAX_VALUE   1200      // максимальная расстройка

#endif
