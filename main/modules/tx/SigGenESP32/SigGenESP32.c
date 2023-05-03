///////////////////////////////////////////////////////////////////////////
//// Driver for ESP32 internal signal generation                    
///////////////////////////////////////////////////////////////////////////

#include "SigGenESP32.h"
#include "board.h"

#include "driver/ledc.h"
#include "esp_err.h"

static const char *TAG = "SIGGENESP32_TX";

// pin configuration
#define F_OUT_GPIO    GPIO_NUM_0



ledc_timer_config_t timer_conf;
ledc_channel_config_t channel_conf;

//*****************************************************************
// sends the 32bit word passed, to the PWM LEDControl 

void setTxFreq(uint32_t fv){  
  timer_conf.freq_hz = fv ; // new frq in kHz
    
  ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));
  ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));     
}


//*****************************************************************
// send the required commands to initialise the PWM LEDControl

void initTX(void){   
   timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE; 
   timer_conf.duty_resolution = LEDC_TIMER_2_BIT;
   timer_conf.timer_num = LEDC_TIMER_0;
   timer_conf.freq_hz = 500;  //    setTxFreq(500);						 //initial frequency
   timer_conf.clk_cfg = LEDC_AUTO_CLK;
   
   ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));
   
   channel_conf.gpio_num = F_OUT_GPIO;
   channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
   channel_conf.channel = LEDC_CHANNEL_0;
   channel_conf.intr_type = LEDC_INTR_DISABLE;
   channel_conf.timer_sel = LEDC_TIMER_0;
   channel_conf.duty = 2;
   channel_conf.hpoint = 0;
   
   ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
   
}

