Installation lvgl-displayd-Skalensimulation

1. Nach dem rekursiven Auschecken des ESP32-ADF/IDF (wenn nicht schon geschehen) das IDF auf Version >=4.3 updaten.

siehe: Update IDF https://docs.espressif.com/projects/esp-idf/en/latest/esp32/versions.html

cd $IDF_PATH
git fetch
git checkout vX.Y.Z
git submodule update --init --recursive

Anmerkung:
* soll der intere I2S->DAC als Audioausgang genutzt werden, bitte (zur Zeit) nur auf v4.3 updaten, sonst ist auch ein Update auf v4.4 möglich.

2. Wechsel in das iRadíoMini-Projektverzeichnis und Installation der lvgl-Bibliothekskomponenten 

git submodule add https://github.com/lvgl/lvgl.git components/lvgl
git submodule add https://github.com/lvgl/lvgl_esp32_drivers.git components/lvgl_esp32_drivers

siehe: https://docs.lvgl.io/8/get-started/espressif.html

Alternativ kann LVGL und das LVGL-Treiberpacket auch vom BM45-github bezogen werden.

3. Anschluß und Konfiguration des Displays in idf.py menuconfig 
   Keine Ressourcenkonflikte verursachen! 

   Sollte das ESP32-Modul zusätzlichen PSRAM besitzen, diesen in menuconfig auch aktivieren + LVGL sagen, 
   daß es die "custom malloc/free" Funktionen nutzen soll.

4. Displayd-Skalensimulation mit lvgl-Unterstützung aktivieren.

4.1 In iRadioMini/main/CMakeLists.txt den Eintrag "./modules/display/lvgl_skalensim/displayd_lvgl_skale.c" hinzufügen.

Beispiel:

set(COMPONENT_SRCS "iRadioMini.c" 
		   "./modules/utils.c"
		   "./modules/player.c"
		   "./modules/gpiod.c"
		   "./modules/gpiod_rotary.c"
		   "./modules/sdcard.c"
		   "./modules/httpd.c"
		   
		   "./modules/display/ssd1306OLED/u8g2_esp32_hal.c"
		   "./modules/display/ssd1306OLED/displayd_i2c.c"

		   "./modules/display/servo/servo.c" 

		   "./modules/display/lvgl_skalensim/displayd_lvgl_skale.c")

set(COMPONENT_ADD_INCLUDEDIRS "./modules" )

register_component()


4.2  in iRadioMini.c einen displayd_lvgl_skale Task starten.
...
     #include "modules/display/lvgl_skalensim/displayd_lvgl_skale.h"
...

     TaskHandle_t xdisplaydHandle = NULL;
     xTaskCreate( displayd_lvgl_skale, "displayd", 4096, NULL , tskIDLE_PRIORITY, &xdisplaydHandle );
     configASSERT(xdisplaydHandle);


5. Firmware compilieren und flashen.


