ESP32 Audioboards mit ES8388 Codec werden durch den Treiber im ADF Audio_HAL nicht optimal initialisiert. 
Folge ist eine viel zu leise Wiedergabelautstärke über den Chip. Um das zu beheben, kompieren Sie den Inhalt von es8388_patch.zip
in den Ablageort Ihrer ESP32-ADF Installation (Ordner esp-adf/components/audio_hal/driver/es8388/). Nach dem Ersetzen der es8388.h 
und es8388.c Dateien müssen Sie das iRadioMini-Projekt nochmals neu compilieren und in den Controller flashen.

