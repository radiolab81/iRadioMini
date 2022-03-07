///////////////////////////////////////////////////////////////////////////
//// Driver for AD9850 DDS chip from Analog Devices                    
///////////////////////////////////////////////////////////////////////////

#include "AD9850.h"
#include "board.h"

#define pulseHigh(pin) { gpio_set_level(pin, 1); gpio_set_level(pin, 0); }

static const char *TAG = "AD9850_TX";

// pin configuration     
#define RST     GPIO_NUM_5 
#define FQ      GPIO_NUM_18
#define CLK     GPIO_NUM_23 
#define DATA    GPIO_NUM_19


void AD9850_Reset(){
  pulseHigh(RST); //Reset Signal
  pulseHigh(CLK); //Clock Signal
  pulseHigh(FQ);  //Frequenz Update Signal
}


void initTX(){
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = ( BIT(RST) | BIT(FQ) | BIT(CLK) | BIT(DATA) );
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   gpio_set_level(RST, 0);
   gpio_set_level(FQ, 0);
   gpio_set_level(CLK, 0);
   gpio_set_level(DATA, 0);

   AD9850_Reset();
}


void AD9850_SendData(unsigned char c) {
  int i;
  
  for(i=0; i<8; i++) {
    gpio_set_level(DATA, (c>>i)&0x01);
    pulseHigh(CLK);
  }
}

void setTxFreq(uint32_t freq){
  long int y;
  
  double frequenz = (double) freq;
  frequenz=frequenz/1000000*4294967295/125; //für ein 125 MHz Quarz
  y=frequenz;
  AD9850_SendData(y);     // w4 - Frequenzdaten LSB übertragen
  AD9850_SendData(y>>8);  // w3
  AD9850_SendData(y>>16); // w2
  AD9850_SendData(y>>24); // w1 - Frequenzdaten MSB
  AD9850_SendData(0x00);  // w0 - 0x00 keine Phase
  pulseHigh(FQ);          // Die neue Frequenz ausgeben
}




