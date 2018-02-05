<h2>Universal Si570/Si5351 VFO</h2>

Oscillator - Si570 and/or Si5351<br>
CPU - Arduino Nano 5v ATMEGA328<br>
Display:<br>
  2.8" SPI TFT ILI9341<br>
  1.8" SPI TFT ST7735<br>
  LCD 1602<br>
Clock & EEPROM - TinyRTC or AT24C32<br>
Keyboard, BPF & other - PCF8574<br>
Rotary encoder - optical 360-400 pulse/turn preffered

Working with single Si570/Si5351 or with dual Si570+Si5351.<br>
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
 1. PDQ GFX with packed font support https://github.com/andrey-belokon/PDQ_GFX_Libs. You need install PDQ_GFX and PDQ_ILI9341 

<img src="doc\ILI8341_570_5351_two_plate\DSC06146.jpg"></img>
<img src="doc\ILI8341_570_5351_two_plate\DSC06150.jpg"></img>

<img src="doc\ILI8341_570_5351_two_plate\Si5351-Syntez-CPU.png"></img>
<img src="doc\ILI8341_570_5351_two_plate\Si5351-Syntez-Interface.png"></img>

Copyright (c) 2016-2018, Andrii Bilokon, UR5FFR
License GNU GPL, see license.txt for more information