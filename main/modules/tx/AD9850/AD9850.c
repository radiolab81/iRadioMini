///////////////////////////////////////////////////////////////////////////
//// Driver for AD9850 DDS chip from Analog Devices                    
///////////////////////////////////////////////////////////////////////////

#include "AD9850.h"
#include "board.h"

#define pulseHigh(pin) { gpio_set_level(pin, 1); vTaskDelay(1/portTICK_PERIOD_MS ); gpio_set_level(pin, 0); }

static const char *TAG = "AD9850_TX";

// pin configuration     
#define RST_LINE     GPIO_NUM_5
#define FQ_LINE      GPIO_NUM_18
#define CLK_LINE     GPIO_NUM_23
#define DATA_LINE    GPIO_NUM_22


void AD9850_Reset(){
  pulseHigh(RST_LINE); //Reset Signal
  pulseHigh(CLK_LINE); //Clock Signal
  pulseHigh(FQ_LINE);  //Frequenz Update Signal
}


void initTX(){
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = ( BIT(RST_LINE) | BIT(FQ_LINE) | BIT(CLK_LINE) | BIT(DATA_LINE) );
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   gpio_set_level(RST_LINE, 0);
   gpio_set_level(FQ_LINE, 0);
   gpio_set_level(CLK_LINE, 0);
   gpio_set_level(DATA_LINE, 0);

   AD9850_Reset();
}


void AD9850_SendData(unsigned char c) {
  int i;
  
  for(i=0; i<8; i++) {
    gpio_set_level(DATA_LINE, (c>>i)&0x01);
    pulseHigh(CLK_LINE);
  }

}

void setTxFreq(uint32_t freq){
  long int y;
  
  double frequenz = (double) freq;
  frequenz=frequenz/1000000*4294967295/125; //für einen 125 MHz Quarz
  y=frequenz;
  AD9850_SendData(y);     // w4 - Frequenzdaten LSB übertragen
  AD9850_SendData(y>>8);  // w3
  AD9850_SendData(y>>16); // w2
  AD9850_SendData(y>>24); // w1 - Frequenzdaten MSB
  AD9850_SendData(0x00);  // w0 - 0x00 keine Phase
  pulseHigh(FQ_LINE);          // Die neue Frequenz ausgeben
}




