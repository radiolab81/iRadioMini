#include "gpiod.h"

#include "board.h"
#include "globals.h"

#include "messages.h"
#include "esp_log.h"

#define KEY_MEDIAPLAYER  GPIO_NUM_23
#define KEY_PRG_NEXT  GPIO_NUM_5
#define KEY_PRG_PREV  GPIO_NUM_18

static const char *TAG = "GPIOD";

void gpiod(void *pvParameters) {
  /* ESP_LOGI(TAG, "***** GPIO-DEFS *****");
  ESP_LOGI(TAG, "VOLDOWN-KEY %i",get_input_voldown_id());
  ESP_LOGI(TAG, "VOLUP-KEY %i",get_input_volup_id());
  ESP_LOGI(TAG, "PLAY-KEY %i",get_input_play_id());
  ESP_LOGI(TAG, "SET-KEY %i",get_input_set_id());
  ESP_LOGI(TAG, "MODE-KEY %i",get_input_mode_id());
  ESP_LOGI(TAG, "REC-KEY %i",get_input_rec_id());
  ESP_LOGI(TAG, "HEADPHONE DETECT GPIO %i",get_headphone_detect_gpio());
  ESP_LOGI(TAG, "AUXIN DETECT GPIO %i",get_auxin_detect_gpio());
  ESP_LOGI(TAG, "GREEN LED GPIO %i",get_green_led_gpio());*/

  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = ( BIT(KEY_PRG_NEXT) | BIT(KEY_PRG_PREV) | BIT(KEY_MEDIAPLAYER) );
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);

  struct AMessage *pxMessage;

  // Daemonbetrieb
  while (1) {
    if (!gpio_get_level(KEY_MEDIAPLAYER)) {
       ESP_LOGI(TAG, "toggle playermodi (internet/mediaplayer)");

       if (MEDIAPLAYER_ENABLED) 
	    xMessage.ucMessage = ENABLE_INTERNETRADIO;
       else 
            xMessage.ucMessage = ENABLE_MEDIAPLAYER;

       pxMessage = &xMessage; 
       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
    } 

    if (!gpio_get_level(KEY_PRG_NEXT))
	 {
	    ESP_LOGI(TAG, "prog+");
	    xMessage.ucMessage = NEXT_PRG;
	    pxMessage = &xMessage; 
	    xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
         }

     if (!gpio_get_level(KEY_PRG_PREV))
	{
	    ESP_LOGI(TAG, "prog-");
	    xMessage.ucMessage = PREV_PRG;
	    pxMessage = &xMessage; 
	    xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
        }
 
      vTaskDelay(500/portTICK_PERIOD_MS );
   }


}
