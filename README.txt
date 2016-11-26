Required libraries: 
* bogde's HX711 library
* Arduino Time library
* adafruit cc3000 library
* Jack Christensen's DS3232RTC library. 


The current sketch that does all the things is: 
Arduino_Load_Cell\Arduino_Sketches\Do_ManyThings\ALLtehTHINGS


required hardware: 
* Arduino Mega 2560 board
* a number of wheatstone bridge load cells
* a number of HX711 amplifiers
* an Adafruit cc3000 wifi shield
* a microSD card (formatted with fat32 or fat16)
* a DS3231 real-time clock
* a coin cell battery (for the clock)
* header pins and jumper wires


Many of the sketches can be run by an Arduino Uno, but the ones that use both wifi and an SD card use too much memory (both program space and SRAM).