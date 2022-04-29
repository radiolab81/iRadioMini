#include "sdcard.h"

#include "esp_log.h"
#include "audio_mem.h"

#include "board.h"
#include <string.h>

#include "utils.h"
#include "globals.h"

#include "sdcard_scan.h"

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


esp_err_t readAMTXConfigSDCard() {
    esp_log_level_set(TAG, ESP_LOG_INFO);       
    TX_ENABLED = false;
    am_tx_freq = 0;

    ESP_LOGI(TAG, "Try read AM.txt from sdcard");
    FILE *fp = fopen("/sdcard/AM.txt", "r");
    if(fp!=NULL) {
      char *read_str = audio_calloc(1, 6); // audio_callc allocates SPI_RAM if available ob audio board
      if (read_str!=NULL) {
        fgets(read_str, 6, fp);
        ESP_LOGI(TAG, "AM.txt: %s", read_str);
	am_tx_freq=atoi(read_str);
        if (am_tx_freq) {
          TX_ENABLED = true;
	  am_tx_freq = am_tx_freq * 1000; // kHz
        }
        free(read_str);
        fclose(fp);
       
        if (TX_ENABLED) {
           return ESP_OK;
	}
      }  
    } else {
      ESP_LOGE(TAG, "No AM.txt on sdcard!");
      return ESP_FAIL;
    }
   
    return ESP_FAIL;
}


/* MEDIAPLAYER BEGIN*/
void sdcard_url_save_cb(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

esp_err_t stop_mediaplayer_service() {
    //MEDIAPLAYER_ENABLED = false;
    ESP_LOGI(TAG, "Release sdcard playlist");
    sdcard_list_destroy(sdcard_list_handle);
    return ESP_OK;
} 

esp_err_t start_mediaplayer_service() {
    ESP_LOGI(TAG, "Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_scan(sdcard_url_save_cb, "/sdcard", 0, (const char *[]) {"mp3", "aac"}, 2, sdcard_list_handle);
    sdcard_list_show(sdcard_list_handle);
   
    if (sdcard_list_get_url_num(sdcard_list_handle)>0) {
      ESP_LOGI(TAG, "mediaplayer service active");
       //MEDIAPLAYER_ENABLED = true;
       return ESP_OK;
    } else {
      ESP_LOGI(TAG, "no files found - mediaplayer service inactive");
      stop_mediaplayer_service();
      return ESP_FAIL;
    } 
 
}
/* MEDIAPLAYER END*/

