#include "transmitterd.h"
#include "globals.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "globals.h"

#include "messages.h" 

#include "./AD9835/AD9835.h"

static const char *TAG = "TRANSMITTERD";

void transmitterd(void * pvParameters)
{
  esp_log_level_set(TAG, ESP_LOG_INFO);
  struct ATransmitterdMessage *rxMsg; 
  
  initTX();
  setTxFreq(am_tx_freq); 

  for( ;; )
  {
    if(xTransmitterdQueue)
    {
      if( xQueueReceive( xTransmitterdQueue, &( rxMsg ), ( TickType_t ) 10 )  )
      {
        //ESP_LOGD(TAG, "Message to transmitterd: %i",rxMsg->ucMessage);
        
        //  Anforderung Frequenzwechsel
        if (rxMsg->ucMessage==SET_FREQ) {
	   setTxFreq(rxMsg->new_freq);
        } // if (rxMsg->ucMessage==SET_FREQ) {

      } // if( xQueueReceive( xTransmitterdQueue, &( rxMsg ), ( TickType_t ) 10 )  )
   
     vTaskDelay(10/portTICK_PERIOD_MS );
   } // if(xTransmitterdQueue)
  } // for( ;; )
}

