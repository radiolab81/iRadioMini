#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "audio_pipeline.h"
#include "periph_wifi.h"

#define MAX_CHANNELS_IN_LIST 100
char* playlist[MAX_CHANNELS_IN_LIST];
int   channels_in_list;


periph_wifi_cfg_t wifi_cfg;
esp_periph_set_handle_t set;

#endif
