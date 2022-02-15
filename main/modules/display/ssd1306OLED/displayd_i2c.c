#include "displayd_i2c.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"

#include "messages.h"

// SDA - PIN
#define PIN_SDA GPIO_NUM_21

// SCL - PIN
#define PIN_SCL GPIO_NUM_22

// I2C - Adresse
#define I2C_ADRESS 0x78


static const char *TAG = "DISPLAYD_SSD1306_I2C";

void displayd_i2c(void *pvParameters) {
   // Messagequeues 
   struct ADisplaydMessage *rxMsg; // -> vom Playerprozess
   struct AMessage *txMsg; // -> zum Playerprozess

   // init i2c
   u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
   u8g2_esp32_hal.sda  = PIN_SDA;
   u8g2_esp32_hal.scl  = PIN_SCL;
   u8g2_esp32_hal_init(u8g2_esp32_hal);

   u8g2_t u8g2; // a structure which will contain all the data for one display
   // u8g2_Setup_ssd1306_i2c_128x32_univision_f  --> see u8g2/csrc/u8g2.h for other displays
   u8g2_Setup_ssd1306_i2c_128x64_noname_f(
	&u8g2,
	U8G2_R0,
	//u8x8_byte_sw_i2c,
	u8g2_esp32_i2c_byte_cb,
	u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
   u8x8_SetI2CAddress(&u8g2.u8x8,I2C_ADRESS);

   ESP_LOGI(TAG, "u8g2_InitDisplay");
   u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

   ESP_LOGI(TAG, "u8g2_SetPowerSave");
   u8g2_SetPowerSave(&u8g2, 0); // wake up display

   char buffer[10];
 
   // Daemonbetrieb
   while(1)
    {
         xMessage.ucMessage = GET_CHANNEL_INFO;
         txMsg = &xMessage; 
	 xQueueSend( xPlayerQueue, ( void * ) &txMsg, ( TickType_t ) 0 );

	 if( xQueueReceive( xDisplaydQueue, &( rxMsg ), ( TickType_t ) 10 )  )
          {
            if (rxMsg->ucMessage==GET_CHANNEL_INFO) {
		ESP_LOGI(TAG, "actual channel num: %i",rxMsg->uciChannelNum);
                ESP_LOGI(TAG, "music_info.sample_rates: %i",rxMsg->music_info.sample_rates);	
		ESP_LOGI(TAG, "music_info.channels: %i",rxMsg->music_info.channels);	
		ESP_LOGI(TAG, "music_info.bits: %i",rxMsg->music_info.bits);	
		ESP_LOGI(TAG, "music_info.bps: %i",rxMsg->music_info.bps);	
		ESP_LOGI(TAG, "music_info.duration: %i",rxMsg->music_info.duration);	
		ESP_LOGI(TAG, "music_info.uri: %s",rxMsg->ucURI);	
	
            } 
          }
 
          ESP_LOGI(TAG, "u8g2_ClearBuffer");
	  u8g2_ClearBuffer(&u8g2);
        
          ESP_LOGI(TAG, "u8g2_SetFont"); 
          
          // Kanalnummer
	  u8g2_SetFont(&u8g2, u8g2_font_10x20_tr);
          u8g2_DrawStr(&u8g2, 2,17,"P");
          itoa(rxMsg->uciChannelNum,buffer,10);
	  u8g2_DrawStr(&u8g2, 14,17,buffer);
          
          // URI
          u8g2_SetFont(&u8g2, u8g2_font_8x13_tr); // more fonts in /u8g2/tools/font/build/single_font_files
	  u8g2_DrawStr(&u8g2, 2,30,rxMsg->ucURI);

          // BPS
          itoa(rxMsg->music_info.bps,buffer,10);
	  u8g2_DrawStr(&u8g2, 2,45,"BPS:");
  	  u8g2_DrawStr(&u8g2, 60,45,buffer);
         
          // Samplerate
          itoa(rxMsg->music_info.sample_rates,buffer,10);
	  u8g2_DrawStr(&u8g2, 2,60,"SR:");
  	  u8g2_DrawStr(&u8g2, 60,60,buffer);     
   

	  u8g2_SendBuffer(&u8g2);
  
        ESP_LOGI(TAG, "Heartbeat");
        vTaskDelay(1000/portTICK_PERIOD_MS ); 
    }

	
	vTaskDelete(NULL);
}

