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
extern char* playlist[MAX_CHANNELS_IN_LIST];
extern int   channels_in_list;


// set by wifi.txt and playlist.m3u from SDCARD
extern periph_wifi_cfg_t wifi_cfg;
extern esp_periph_set_handle_t set;

// set by AM.txt form SDCARD
extern bool TX_ENABLED;
extern uint32_t am_tx_freq;

// mediaplayerd globals
extern bool MEDIAPLAYER_ENABLED; 
extern playlist_operator_handle_t sdcard_list_handle;

#endif
