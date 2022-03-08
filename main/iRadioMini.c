#include "globals.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "board.h"
#include "modules/sdcard.h"

#include "modules/player.h"
#include "modules/messages.h"

#include "modules/gpiod.h"
#include "modules/gpiod_rotary.h"

#include "modules/httpd.h"

#include "modules/display/ssd1306OLED/displayd_i2c.h"
#include "modules/display/HD44780_I2C/displayd_HD44780_i2c.h"

#include "modules/display/servo/servo.h"

#include "modules/tx/transmitterd.h"

#if __has_include("esp_idf_version.h")
#include "esp_idf_version.h"
#else
#define ESP_IDF_VERSION_VAL(major, minor, patch) 1
#endif

#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
#include "esp_netif.h"
#else
#include "tcpip_adapter.h"
#endif

static const char *TAG = "IRADIOMINI";


void app_main(void)
{
   esp_log_level_set(TAG, ESP_LOG_INFO);
 
   // BOARDINITIALISIERUNGEN
   ESP_LOGI(TAG, "call startup procedures (aka rc.local)");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0))
    ESP_ERROR_CHECK(esp_netif_init());
#else
    tcpip_adapter_init();
#endif

    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    set = esp_periph_set_init(&periph_cfg);

    // Einlesen der wifi.txt und playlist.m3u von SD-Karte
    audio_board_sdcard_init(set, SD_MODE_1_LINE);
    wifi_cfg = readWifiConfigSDCard();
    readPlaylistSDCard();
    
    // Playerprozess starten
    ESP_LOGI(TAG, "prepare to start the player process");
    TaskHandle_t xPlayerTaskHandle = NULL;
    xTaskCreate( player, "player", 4096, NULL, tskIDLE_PRIORITY, &xPlayerTaskHandle );
    configASSERT(xPlayerTaskHandle);

    // Queues für Nachrichtenaustausch zwischen den Prozessen
    ESP_LOGI(TAG, "start IPC");
    // - control queue of player process
    xPlayerQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if (!xPlayerQueue)
      ESP_LOGE(TAG, "IPC:Player Message Queue failed");

    // - displayd queue 
    xDisplaydQueue = xQueueCreate( 10, sizeof( struct ADisplaydMessage * ) );
    if (!xDisplaydQueue)
      ESP_LOGE(TAG, "IPC:Displayd Message Queue failed");
   
   
    // Kontrollprozess für Player starten
    TaskHandle_t xPlayerControlTaskHandle = NULL;
    xTaskCreate( playerControlTask, "playerControlTask", 4096, NULL , tskIDLE_PRIORITY, &xPlayerControlTaskHandle );
    configASSERT(xPlayerControlTaskHandle);
 
    // Suchen + Einlesen der AM.txt 
    /*if (readAMTXConfigSDCard()==ESP_OK) {
      if (TX_ENABLED) {
        vTaskDelay(5000/portTICK_PERIOD_MS );
	ESP_LOGI(TAG, "start AMTX-Daemon on %i kHz...",am_tx_freq/1000);
        // - control queue of transmitterd process
        xTransmitterdQueue = xQueueCreate( 10, sizeof( struct ATransmitterdMessage * ) );
        if (!xTransmitterdQueue) {
          ESP_LOGE(TAG, "IPC:Transmitterd Message Queue failed");
	}
 
        TaskHandle_t xTransmitterdTaskHandle = NULL;
        xTaskCreate( transmitterd, "transmitterd", 4096, NULL , tskIDLE_PRIORITY, &xTransmitterdTaskHandle );
        configASSERT(xTransmitterdTaskHandle);

        //TaskHandle_t xinet2RDdTaskHandle = NULL;
        //xTaskCreate( inet2RFd, "inet2RFd", 4096, NULL , tskIDLE_PRIORITY, &xinet2RDdTaskHandle );
        //configASSERT(xinet2RDdTaskHandle);
       } 
    }*/

    // Prozess gpiod starten
    TaskHandle_t xgpiodHandle = NULL;
    xTaskCreate( gpiod, "gpiod", 2048, NULL , tskIDLE_PRIORITY, &xgpiodHandle );
    configASSERT(xgpiodHandle);

    // Prozess gpiod_rotary starten
    // TaskHandle_t xgpiodHandle = NULL;
    // xTaskCreate( gpiod_rotary, "gpiod", 2048, NULL , tskIDLE_PRIORITY, &xgpiodHandle );
    // configASSERT(xgpiodHandle);
 
    // Prozess displayd starten
    TaskHandle_t xdisplaydHandle = NULL;
    xTaskCreate( displayd_i2c, "displayd", 4096, NULL , tskIDLE_PRIORITY, &xdisplaydHandle );
    configASSERT(xdisplaydHandle);

    // Prozess servod starten
    //TaskHandle_t xdisplaydHandle = NULL;
    //xTaskCreate( displayd_servo, "displayd", 2048, NULL , tskIDLE_PRIORITY, &xdisplaydHandle );
    //configASSERT(xdisplaydHandle);

    // Prozess httpd starten
    TaskHandle_t xhttpdHandle = NULL;
    xTaskCreate( httpd, "httpd", 4096, NULL , tskIDLE_PRIORITY, &xhttpdHandle );
    configASSERT(xhttpdHandle);

    // Daemonbetrieb - Hauptprozess
  
    while(1)
    {

      ESP_LOGI(TAG, "Heartbeat");
      vTaskDelay(1000/portTICK_PERIOD_MS );
         
    }

   
    ESP_LOGI(TAG, "cleanup");
    if(xPlayerTaskHandle)
      vTaskDelete(xPlayerTaskHandle);

    if(xPlayerControlTaskHandle)
      vTaskDelete(xPlayerControlTaskHandle);
  
    if(xgpiodHandle)
      vTaskDelete(xgpiodHandle);

    if(xdisplaydHandle)
      vTaskDelete(xdisplaydHandle);

    if(xhttpdHandle)
      vTaskDelete(xhttpdHandle);

 /* if(xTransmitterdTaskHandle)
      vTaskDelete(xTransmitterdTaskHandle); */
	
 /* if(xinet2RDdTaskHandle)
      vTaskDelete(xinet2RDdTaskHandle); */

    esp_periph_set_destroy(set);
}
