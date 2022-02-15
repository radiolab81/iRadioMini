#include "sdcard.h"

#include "esp_log.h"
#include "audio_mem.h"

#include "board.h"
#include <string.h>

#include "utils.h"
#include "globals.h"

static const char *TAG = "SDCARD";
    
periph_wifi_cfg_t readWifiConfigSDCard() {
    esp_log_level_set(TAG, ESP_LOG_INFO);       

    ESP_LOGI(TAG, "Try read wifi.txt from sdcard");
    char* ssid = calloc(80,sizeof(char));
    char* psk = calloc(80,sizeof(char));
    FILE *fp = fopen("/sdcard/wifi.txt", "r");
    if(fp!=NULL) {
      char *read_str = audio_calloc(1, 512); // audio_calloc allocates SPI_RAM if available ob audio board
      if (read_str!=NULL) {
         fread(read_str, 1, 512, fp);
         ESP_LOGI(TAG, "wifi.txt\n: %s", read_str);
     
	 // SSID
         char *ssid_begin = (strstr(read_str,"ssid=")+6*sizeof(char)); 
	 if (ssid_begin!=NULL) {
  	    char* ssid_end = strstr(ssid_begin,"\"");
	    if (ssid_end!=NULL) {
               if (memcpy(ssid,ssid_begin,(ssid_end-ssid_begin))) 
		 ESP_LOGI(TAG, "SSID found: %s", ssid);
	    }
	 }

	 // PSK
         char *psk_begin = (strstr(read_str,"psk=")+5*sizeof(char)); 
	 if (psk_begin!=NULL) {
  	    char* psk_end = strstr(psk_begin,"\"");
	    if (psk_end!=NULL) {
               if (memcpy(psk,psk_begin,(psk_end-psk_begin))) 
		  ESP_LOGI(TAG, "PSK found: %s", psk);
	    }
	 }

        free(read_str);
        fclose(fp);
      }  
    } else {
      ESP_LOGE(TAG, "No wifi.txt on sdcard!");
    }

    periph_wifi_cfg_t wifi_cfg = {
        .ssid = ssid,
        .password = psk,
    };
    
    return wifi_cfg;
}

void readPlaylistSDCard() {       
    for (int i=0; i<MAX_CHANNELS_IN_LIST; i++) playlist[i]=NULL;
    channels_in_list = 0;

    ESP_LOGI(TAG, "Try read channel list from sdcard");
    FILE *fp = fopen("/sdcard/playlist.m3u", "r");
    if(fp!=NULL) {
      char url[255];
      int i=0;

      while(fgets(url,255,fp) && (i<MAX_CHANNELS_IN_LIST)) {
        playlist[i]=malloc(255*sizeof(char));
	memcpy(playlist[i],url,255);
        // rm possible control chars
        rmSubstr(playlist[i],"\r");
        rmSubstr(playlist[i],"\n");
        rmSubstr(playlist[i]," ");
        ESP_LOGI(TAG, "read URL:%s:%i", playlist[i],strlen(playlist[i]));
 	i++;
      } // while(fgets(url,255,fp)) {
     channels_in_list = i;
     fclose(fp);
    } // if(fp!=NULL) {
    else {
      ESP_LOGE(TAG, "No playlist.m3u on sdcard!");
    }

} 
