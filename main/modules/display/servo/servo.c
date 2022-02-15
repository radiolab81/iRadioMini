#include "servo.h"
#include "globals.h"
#include "messages.h" 

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>


#define SERVO_PIN GPIO_NUM_23         //   PWM-Anschluß zum Servo
#define TRIGGER_SERVO_PIN GPIO_NUM_19 //   Freigabe der Versorgungsspannung Servo

#define GESAMT_PULSBREITE 1024        //  LEDC_TIMER_10_BIT->2^10 = 1024

#define MIN_SERVO_PULSBREITE 1 		        // Zeigeranschlag links
#define MAX_SERVO_PULSBREITE GESAMT_PULSBREITE  // Zeigeranschlag rechts

#define SENDERABSTAND (MAX_SERVO_PULSBREITE-MIN_SERVO_PULSBREITE)/channels_in_list

static const char *TAG = "DISPLAYD_SERVO";
int old_ch = -1;

void displayd_servo(void *pvParameters) {
   // Messagequeues 
   struct ADisplaydMessage *rxMsg; // -> vom Playerprozess
   struct AMessage *txMsg; // -> zum Playerprozess

   ledc_timer_config_t timer_conf;
   timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
   timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
   timer_conf.timer_num = LEDC_TIMER_0;
   timer_conf.freq_hz = 50;
   //timer_conf.clk_cfg = LEDC_AUTO_CLK;
   ledc_timer_config(&timer_conf);

   ledc_channel_config_t ledc_conf;
   ledc_conf.gpio_num = SERVO_PIN;
   ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
   ledc_conf.intr_type = LEDC_INTR_DISABLE;
   ledc_conf.channel = LEDC_CHANNEL_0;
   ledc_conf.timer_sel = LEDC_TIMER_0;
   ledc_conf.duty = MIN_SERVO_PULSBREITE;
   ledc_channel_config(&ledc_conf);

   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = BIT(TRIGGER_SERVO_PIN);
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

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
                if (old_ch!=rxMsg->uciChannelNum) {
                   ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, MIN_SERVO_PULSBREITE+(rxMsg->uciChannelNum*SENDERABSTAND));
		   ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	
		   gpio_set_level(TRIGGER_SERVO_PIN,1);
		   vTaskDelay(2000/portTICK_PERIOD_MS);
		   gpio_set_level(TRIGGER_SERVO_PIN,0);
                } // if (old_ch!=rxMsg->uciChannelNum) {
 	       old_ch = rxMsg->uciChannelNum;
            }
        }
            
        ESP_LOGI(TAG, "Heartbeat");
        vTaskDelay(1000/portTICK_PERIOD_MS ); 
    }
	
  vTaskDelete(NULL);
}
