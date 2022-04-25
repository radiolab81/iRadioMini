#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "audio_pipeline.h"
#include "periph_wifi.h"
#include "sdcard_list.h"

//#define USE_INTERNAL_AUDIODAC

// override by menuconfig
#ifdef CONFIG_USE_INTERNAL_AUDIODAC
  #define USE_INTERNAL_AUDIODAC
#endif

#define MAX_CHANNELS_IN_LIST 100
char* playlist[MAX_CHANNELS_IN_LIST];
int   channels_in_list;


// set by wifi.txt and playlist.m3u from SDCARD
periph_wifi_cfg_t wifi_cfg;
esp_periph_set_handle_t set;

// set by AM.txt form SDCARD
bool TX_ENABLED;
uint32_t am_tx_freq;

// mediaplayerd globals
bool MEDIAPLAYER_ENABLED; 
playlist_operator_handle_t sdcard_list_handle;

#endif
