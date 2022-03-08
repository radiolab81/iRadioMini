#include "inet2RFd.h"

#include "globals.h"
#include "board.h"

#include "messages.h"
#include "esp_log.h"

#include "soc/pcnt_struct.h"
#include "driver/pcnt.h"

#include "esp_timer.h"
#include "esp_sleep.h"

static const char *TAG = "INET2RF";

#define FREQ_IN_CHAN_GPIO GPIO_NUM_19   // GPIO for frequency measurement
#define IF_OFFSET 0                     // intermediate frequency offset in kHz, normally 455, 450, ... 

// Eckfrequenzen des RF Bereichs in den uebersetzt werden soll  
#define MINIMAL_TX_FRQ 	526		
#define MAXIMAL_TX_FRQ  1606

#define SENDERABSTAND (MAXIMAL_TX_FRQ-MINIMAL_TX_FRQ)/channels_in_list


////////////////////////////////////////////////////////////////////////////////////////
#define PCNT_HIGH_LIMIT 20000
#define PCNT_LOW_LIMIT  0

#define USE_PCNT_UNIT PCNT_UNIT_0
#define USE_PCNT_CH   PCNT_CHANNEL_0

portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;
uint16_t overflow_counter = 0;

void IRAM_ATTR pcnt_event_handler(void* arg){
    portENTER_CRITICAL_ISR(&timer_mux);
    overflow_counter++;
    PCNT.int_clr.val = BIT(USE_PCNT_UNIT); 
    portEXIT_CRITICAL_ISR(&timer_mux);
}


volatile double frequency = 0;
void pcnt_get_frequency()
{ 
    uint16_t result = 0; 
    pcnt_counter_pause(USE_PCNT_UNIT); 
    pcnt_get_counter_value(USE_PCNT_UNIT, (int16_t*) &result); 
    frequency =  result + (overflow_counter*20000); 
    overflow_counter = 0; 
    pcnt_counter_clear(USE_PCNT_UNIT); 
    pcnt_counter_resume(USE_PCNT_UNIT);
}


static void periodic_timer_callback(void* arg)
{
    pcnt_get_frequency();
    //int64_t time_since_boot = esp_timer_get_time();
    //ESP_LOGI(TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);
}

int getChannelNoFromFreq(double frequency) {
    for (int i=0; i<channels_in_list; i++) {
         if ( (frequency > ((MINIMAL_TX_FRQ + i*SENDERABSTAND) - (SENDERABSTAND/2)) ) &&
              (frequency < ((MINIMAL_TX_FRQ + i*SENDERABSTAND) + (SENDERABSTAND/2)) ) ) {
           return i; 
         }
    }
    return 0;
}

void inet2RFd(void *pvParameters) {

   pcnt_config_t unit_config = {
      .counter_h_lim = PCNT_HIGH_LIMIT,      // set upper counter limit
      .counter_l_lim = PCNT_LOW_LIMIT,       // set lower counter limit
      .pulse_gpio_num = FREQ_IN_CHAN_GPIO,   // used input pin for counter
      .ctrl_gpio_num = -1,		     // no control pin
      .pos_mode = PCNT_COUNT_INC,	     // inc counter on rising edge
      .neg_mode = PCNT_COUNT_DIS,	     // do nothing on falling edge
      .lctrl_mode = PCNT_MODE_KEEP,          // control pin not in use -> do nothing on falling edge of control pin
      .hctrl_mode = PCNT_MODE_KEEP,          // control pin not in use -> do nothing on rising edge of control pin
      .channel = USE_PCNT_CH,			
      .unit = USE_PCNT_UNIT
   };

   pcnt_unit_config(&unit_config);
   pcnt_event_enable(USE_PCNT_UNIT, PCNT_EVT_H_LIM);  // enable event on upper counter limit overflow
   pcnt_isr_register(pcnt_event_handler, NULL, 0, NULL); // register ISR event handler 
   pcnt_intr_enable(USE_PCNT_UNIT);

   pcnt_counter_pause(USE_PCNT_UNIT); 
   pcnt_counter_clear(USE_PCNT_UNIT);
   pcnt_counter_resume(USE_PCNT_UNIT);
   
   /* create 100ms-timer */
   esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            .name = "periodic"
    };

   esp_timer_handle_t periodic_timer;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

   /* Start the 100ms timer for freq measure */
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 100000));

   struct ATransmitterdMessage *pxTransmitterdMessage;
   struct AMessage *pxMessage;
   
   uint8_t old_ch = 0;

   xTransmitterdMessage.ucMessage = SET_FREQ;
   xTransmitterdMessage.new_freq = (uint32_t) (1000*MINIMAL_TX_FRQ);
   pxTransmitterdMessage = &xTransmitterdMessage; 
   xQueueSend( xTransmitterdQueue, ( void * ) &pxTransmitterdMessage, ( TickType_t ) 0 );

   // Daemonbetrieb
   while (1) {

      ESP_LOGI(TAG, "frequency on GPIO%i is %f kHz",FREQ_IN_CHAN_GPIO,frequency/100); 
      ESP_LOGI(TAG, "frequency is ChannelNo %i",getChannelNoFromFreq((frequency/100)+IF_OFFSET));

      //ESP_LOGI(TAG, "frequency on GPIO%i is %f kHz",FREQ_IN_CHAN_GPIO,f_help/100); 
      //ESP_LOGI(TAG, "frequency is ChannelNo %i",getChannelNoFromFreq((f_help/100)+IF_OFFSET));

        // get local oscillator freq for channel switching -> new channel 
        if (getChannelNoFromFreq((frequency/100)+IF_OFFSET) != old_ch) {

            // set tx to new destination carrier freq
            ESP_LOGI(TAG, "set new tx freq");
            xTransmitterdMessage.ucMessage = SET_FREQ;
            xTransmitterdMessage.new_freq = (uint32_t) (1000*(MINIMAL_TX_FRQ + getChannelNoFromFreq((frequency/100)+IF_OFFSET)*SENDERABSTAND));
	    pxTransmitterdMessage = &xTransmitterdMessage; 
	    xQueueSend( xTransmitterdQueue, ( void * ) &pxTransmitterdMessage, ( TickType_t ) 0 );

            // switch the audio modulation 
            if (old_ch<getChannelNoFromFreq((frequency/100)+IF_OFFSET)) {
               ESP_LOGI(TAG, "prog+");
	       xMessage.ucMessage = NEXT_PRG;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 ); 
	    } else {
               ESP_LOGI(TAG, "prog-");
	       xMessage.ucMessage = PREV_PRG;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

        }

       old_ch = getChannelNoFromFreq((frequency/100)+IF_OFFSET);
       vTaskDelay(300/portTICK_PERIOD_MS ); 

   }

   ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
   ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));

   pcnt_intr_disable(USE_PCNT_UNIT);
   pcnt_event_disable(USE_PCNT_UNIT, PCNT_EVT_H_LIM);


}
