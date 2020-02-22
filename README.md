<h2>Universal Digital VFO Si570/Si5351</h2>

Oscillator - Si570 and/or Si5351<br>
CPU - Arduino Nano 5v ATMEGA328<br>
Display:<br>
  2.8" SPI TFT ILI9341<br>
  1.8" SPI TFT ST7735<br>
  LCD 1602<br>
Realtime clock - DS3231(preffered) or DS1307(TinyRTC) or PCF8563<br>
Keyboard, BPF & other - PCF8574<br>
Rotary encoder - optical 360-400 pulse/turn or mechanical 20 pulse/turn

Allow any combination of two Si5351 and one Si570: Si5351, Si570, Si5351+Si5351, Si570+Si5351, Si570+Si5351+Si5351.<br>
Dual VFO A/B, SPLIT, RIT.<br>
Save current freq and band to EEPROM. <br>
CAT-enabled (Kenwood protocol).<br><br>
Support different TRX architecture:<br>
 1. Single and double IF superheterodyne.
 2. Up-conversion with general coverage 2-30MHz and high IF.
 3. Direct conversion with 2x or 4x output.
 4. Direct conversion with quadrature output.

Project homepage http://dspview.com/viewtopic.php?t=174

Required libraries:<br>
 1. PDQ GFX with packed font support https://github.com/andrey-belokon/PDQ_GFX_Libs. You need install PDQ_GFX, PDQ_ILI9341 and PDQ_ST7735<br>
 2. UR5FFR_Si5351 https://github.com/andrey-belokon/UR5FFR_Si5351

<img src="doc\ST7735_570_5351_two_plate\img\synt_1.8_11.jpg"></img>
<img src="doc\ST7735_570_5351_two_plate\img\synt_1.8_9.jpg"></img>
<img src="doc\ST7735_570_5351_two_plate\img\synt_1.8_4.jpg"></img>
<img src="doc\ST7735_570_5351_two_plate\img\synt_1.8_7.jpg"></img>

<img src="doc\ST7735_570_5351_two_plate\Schematic Digital VFO 1.8 main board 1.2.png"></img>
<img src="doc\ST7735_570_5351_two_plate\Schematic Digital VFO 1.8 display board 1.0.png"></img>

Copyright (c) 2016-2020, Andrii Bilokon, UR5FFR
License GNU GPL, see license.txt for more information