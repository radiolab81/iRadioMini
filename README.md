# iRadioMini für ESP32
Ein Softwarebaukasten für den Aufbau neuer Radios oder Umbau alter Radios zu einem Internetradio auf ESP32 ADF-Basis.

Das iRadioMini ist eine Portierung unseres iRadio für Raspberry (https://github.com/BM45/iRadio) auf ESP32. 

## Unterstützte Boards:

Das iRadioMini für ESP32 unterstützt alle ESP32 Boards aus dem Espressif Audio Development Framework

![adfboard1](https://github.com/BM45/iRadioMini/blob/master/pics4www/esp32-audiokit.jpg)

![adfboard2](https://github.com/BM45/iRadioMini/blob/master/pics4www/boards.jpg)

sowie natürlich auch zahlreiche Nachbauten dieser, mit einem der bastlerfreundlichen ESP32-WROOM 32/WROVER Boards.

![adfboard3](https://github.com/BM45/iRadioMini/blob/master/pics4www/ESP32.jpg)

Auch komplette eigene ESP32 Audio Boards sind natürlich möglich, sofern das eigene Design als Board im ADF hinterlegt wird.

## Steuerung des iRadioMini durch:

Programmumschaltung / Lautstärkeänderung mit (Micro-)Taster, Inkrementaldrehgeber, Drehimpulsgeber oder fernsteuerbar über HTML-Bedienoberfläche. 

Die Steuerung wird wie beim iRadio für Raspberry als Daemon/Prozess "gpiod" realisiert.

## Unterstützte Displays für Nutzerschnittstelle:
- HD44780-kompatible Displays (auch über I2C mit PCF8574/5)

- über U8g2 (U8g2 is a monochrome graphics library for embedded devices.)

 SSD1305, SSD1306, SSD1309, SSD1316, SSD1322, SSD1325, SSD1327, SSD1329, SSD1606, SSD1607, SH1106, SH1107, SH1108, SH1122, T6963, RA8835, LC7981, PCD8544, PCF8812, HX1230, UC1601, UC1604, UC1608, UC1610, UC1611, UC1617, UC1701, ST7511, ST7528, ST7565, ST7567, ST7571, ST7586, ST7588, ST75256, ST75320, NT7534, ST7920, IST3020, IST7920, LD7032, KS0108, KS0713, SED1520, SBN1661, IL3820, MAX7219 (siehe https://github.com/olikraus/u8g2/wiki/u8g2setupcpp für eine vollständige Liste)

![adisplay1](https://github.com/BM45/iRadioMini/blob/master/pics4www/display1.jpg)

- über LVGL kommen weitere farbige Displays, Touchscreen-Controller und ein GUI-System hinzu

![lvgl1](https://github.com/BM45/iRadioMini/blob/master/pics4www/lvgl_main.jpg)
![lvgl2](https://github.com/BM45/iRadioMini/blob/master/pics4www/lvgl_disp.jpg)
![lvgl3](https://github.com/BM45/iRadioMini/blob/master/pics4www/lvgl_indev.jpg)

Ein besonderes Highlight ist die Simulation photorealistischer Nachbildungen von Senderskalen alter Radios,

![lvgl4](https://github.com/BM45/iRadioMini/blob/master/pics4www/ESP32Skalensim.jpg)
![lvgl5](https://github.com/BM45/iRadioMini/blob/master/pics4www/MendeWSim.jpg)

oder eine animierten Kassettensimulation zum Einbau eines Displays in alte Kassettenrecoder.

![lvgl6](https://github.com/BM45/iRadioMini/blob/master/pics4www/cass_sim.jpg)


- PWM-Servo getriebene Analogskalen, zum Beispiel für Programmplatz und Lautstärke, sind eine weitere Möglichkeit.

[![servovideo](http://img.youtube.com/vi/fL3GbyHzpOE/0.jpg)](http://www.youtube.com/watch?v=fL3GbyHzpOE "")

## Unterstützte Audio-DACs:

Zur Zeit werden alle vom ESP-ADF unterstützten Audio-DACs direkt auch im iRadioMini unterstützt. 
Zusätzlich können die internen DACs des ESP32 und GPIO-Pins zur PDM/PWM-Audioausgabe genutzt werden.

## Aussendung des Internetradioprogramms über DDS/SDR:


![tx1](https://github.com/BM45/iRadioMini/pics4www/iRadioMiniTX.jpg)


# Installation:

Das iRadioMini benötigt als Grundvoraussetzung eine Installation des Espressif ADF und IDF. Diese Frameworks können von den offziellen Quellen, wie auch aus meinem BM45-github bezogen werden. ( https://github.com/BM45/esp-adf / https://github.com/BM45/esp-idf )

Nach dem Clonen des iRadioMini für ESP32 in die lokale Entwicklungsumgebung, kann die Firmware mit 

`idf.py build` 

kompiliert werden. Die Übertragung der Firmware auf das ESP32 Audio Board geht mit


`idf.py flash` .

Die Consolen und Debugausgaben können mit 

`idf.py monitor` 

betrachtet werden.

## WiFi-Zugangsdaten und Senderliste:

#### WiFi 
Es kann eine wifi.txt (mit gleichem Inhalt wie wpa_supplicant.conf des iRadio für Raspberry)
in das Rootverzeichnis einer FAT-formatierten SD-Karte abgelegt werden. Nach einem Neustart des Audio Boards werden die WiFi-Zugangsdaten von der Datei wifi.txt verwendet.

#### Senderliste

Es wird eine Datei playlist.m3u in das Rootverzeichnis einer FAT-formatierten SD-Karte abgelegt. 
Nach einem Neustart wird die Senderliste von der SD-Karte automatisch eingelesen und verwendet.
Achtung: In der playlist.m3u darf pro Zeile nur die URL einer Internetradiostation stehen. Keine M3U-Metadaten oder verschachtelte Playlisten verwenden!
Standardmäßig werden Internetradiostreams in den Formaten MP3, AAC, M4A, WAV akzeptiert. 
Weitere Formatedekoder (FLAC, OGG) können innerhalb der Software aktiviert werden.

_____________________________________________________________________________________
Weiterer Support im Radio-Bastler-Forum unter: https://www.radio-bastler.de 

Bei Fragen meldet Euch einfach im Radio-Bastler-Forum an.
