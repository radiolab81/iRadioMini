#include "globals.h"

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

// Equalizer gain settings
int equalizer_gain[20];
