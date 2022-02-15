# iRadioMini für ESP32
Ein Softwarebaukasten für den Aufbau neuer Radios oder dem Umbau alter Radios zu einem Internetradio auf ESP32 ADF-Basis.

Das iRadioMini ist eine Portierung unseres iRadio für Raspberry (https://github.com/BM45/iRadio) auf ESP32. 

## Unterstützte Boards:

Das iRadioMini für ESP32 unterstützt alle ESP32 Boards aus dem Espressif Audio Development Framework. 


![adfboard1](https://user-images.githubusercontent.com/48355256/154087028-28075443-a0d8-4d40-ab4b-7efa5c7f012f.png)


## Steuerung des iRadioMini durch:

Programmumschaltung / Lautstärkeänderung mit (Micro-)Taster, Inkrementaldrehgeber, Drehimpulsgeber oder fernsteuerbar über HTML-Bedienoberfläche. 

Die Steuerung wird wie beim iRadio für Raspberry als Daemon/Prozess "gpiod" realisiert.

## Unterstützte Displays für Nutzerschnittstelle:

- über U8g2 (U8g2 is a monochrome graphics library for embedded devices.)

 SSD1305, SSD1306, SSD1309, SSD1316, SSD1322, SSD1325, SSD1327, SSD1329, SSD1606, SSD1607, SH1106, SH1107, SH1108, SH1122, T6963, RA8835, LC7981, PCD8544, PCF8812, HX1230, UC1601, UC1604, UC1608, UC1610, UC1611, UC1617, UC1701, ST7511, ST7528, ST7565, ST7567, ST7571, ST7586, ST7588, ST75256, ST75320, NT7534, ST7920, IST3020, IST7920, LD7032, KS0108, KS0713, SED1520, SBN1661, IL3820, MAX7219 (siehe https://github.com/olikraus/u8g2/wiki/u8g2setupcpp für eine vollständige Liste)
 
- PWM-Servo getriebene Analogskale zum Beispiel für Programmplatz und Lautstärke

# Installation:

Das iRadioMini benötigt als Grundvoraussetzung eine Installation des Espressif ADF und IDF. Diese Frameworks können von den offziellen Quellen, wie auch aus meinem BM45-github bezogen werden. ( https://github.com/BM45/esp-adf / https://github.com/BM45/esp-idf )

Nach dem Clonen des iRadioMini für ESP32 in die lokale Arbeitskopie, kann die Firmware mit 

`idf.py build` 

kompiliert werden. Die Übertragung der Firmware auf das ESP32 Audio Board geht mit


`idf.py flash` .

Die Consolen und Debugausgaben können mit 

`idf.py monitor` 

betrachtet werden.

