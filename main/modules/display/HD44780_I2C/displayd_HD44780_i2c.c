#include "displayd_HD44780_i2c.h"

#include "board.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#include "messages.h"

#include "hd44780.h"
#include "pcf8574.h"

// I2C - Adresse
#define I2C_ADRESS 0x27 

#define I2C_CONTROLLER_NUM I2C_NUM_1
// SDA - PIN
#define PIN_SDA GPIO_NUM_21
// SCL - PIN
#define PIN_SCL GPIO_NUM_22
// or use ADF API to get I2C-Pins by ADF board def. 
   // ESP_ERROR_CHECK(get_i2c_pins(I2C_NUM_1, &i2c_cfg));
   // i2c_cfg.mode = I2C_MODE_MASTER;
   // i2c_cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
   // i2c_cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;

static const char *TAG = "DISPLAYD_HD44780_I2C";

static  i2c_dev_t pcf8574;  

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);                              
}

void displayd_HD44780_i2c(void *pvParameters) {
   
   // Messagequeues 
   struct ADisplaydMessage *rxMsg; // -> vom Playerprozess
   struct AMessage *txMsg; // -> zum Playerprozess
   int old_ch = -1;

   // init  
   hd44780_t lcd = {
        .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
        .font = HD44780_FONT_5X8,
        .lines = 2, // 4
        .pins = {
            .rs = 0,
            .e  = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3
        }
   };

   memset(&pcf8574, 0, sizeof(i2c_dev_t));
   pcf8574.cfg.scl_pullup_en = true;
   pcf8574.cfg.sda_pullup_en = true;
   ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, I2C_CONTROLLER_NUM , I2C_ADRESS, PIN_SDA, PIN_SCL));
   ESP_ERROR_CHECK(hd44780_init(&lcd));
 
   hd44780_switch_backlight(&lcd, true);
 
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
 
         if (old_ch!=rxMsg->uciChannelNum) {
           hd44780_clear(&lcd);
           hd44780_gotoxy(&lcd, 0, 0);
           hd44780_putc(&lcd, 'P');
           hd44780_gotoxy(&lcd, 1, 0);
           itoa(rxMsg->uciChannelNum,buffer,10);
           hd44780_puts(&lcd, buffer);

           hd44780_gotoxy(&lcd, 0, 1); 
	   if (strlen(rxMsg->ucURI)>20) {
		rxMsg->ucURI[19]='\0';
	   }
           hd44780_puts(&lcd, rxMsg->ucURI);
         } // if (old_ch!=rxMsg->uciChannelNum) {
   
        old_ch = rxMsg->uciChannelNum;
 
  
        ESP_LOGI(TAG, "Heartbeat");
        vTaskDelay(1000/portTICK_PERIOD_MS ); 
    }

   pcf8574_free_desc(&pcf8574);
   vTaskDelete(NULL);
}

