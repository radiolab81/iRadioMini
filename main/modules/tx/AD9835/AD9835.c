///////////////////////////////////////////////////////////////////////////
//// Driver for AD9835 DDS chip from Analog Devices                    
///////////////////////////////////////////////////////////////////////////

#include "AD9835.h"

#include "board.h"


static const char *TAG = "AD9835_TX";


// pin configuration
#define FSYNC    GPIO_NUM_5
#define SCLK     GPIO_NUM_18
#define SDATA    GPIO_NUM_23


#define SCLK_HIGH gpio_set_level(SCLK,1)
#define SCLK_LOW gpio_set_level(SCLK,0)
#define FSYNC_HIGH gpio_set_level(FSYNC,1)
#define FSYNC_LOW gpio_set_level(FSYNC,0)
#define SDATA_HIGH gpio_set_level(SDATA,1)
#define SDATA_LOW gpio_set_level(SDATA,0)


// commands
#define CMD_WRI16_PHASE_REG      0x0000
#define CMD_WRI8_PHASE_DEFER_REG 0x1000
#define CMD_WRI16_FREQ_REG       0x2000
#define CMD_WRI8_FREQ_DEFER_REG  0x3000
#define CMD_SELECT_PHASE_REG     0x4000
#define CMD_SELECT_FREQ_REG      0x5000
#define CMD_SELECT_PHASE_AND_FREQ_REG 0x6000

// controlling the AD9835
#define CMD_SET_SYNC_AND_SELSRC  0x8000
#define CMD_SET_SLEEP_RST_CLR    0xC000
#define SYNC_HIGH                0x2000
#define SELSRC_HIGH              0x1000
#define SLEEP_HIGH               0x2000
#define RESET_HIGH               0x1000
#define CLR_HIGH                 0x800

// adress of destination registers
#define FREQ0_REG_8_L_LBS 0x000
#define FREQ0_REG_8_H_LBS 0x100
#define FREQ0_REG_8_L_MSB 0x200
#define FREQ0_REG_8_H_MSB 0x300
#define FREQ1_REG_8_L_LBS 0x400
#define FREQ1_REG_8_H_LBS 0x500
#define FREQ1_REG_8_L_MSB 0x600
#define FREQ1_REG_8_H_MSB 0x700
#define PHASE0_REG_8_LSB 0x800
#define PHASE0_REG_8_MSB 0x900
#define PHASE1_REG_8_LSB 0xA00
#define PHASE1_REG_8_MSB 0xB00
#define PHASE2_REG_8_LSB 0xC00
#define PHASE2_REG_8_MSB 0xD00
#define PHASE3_REG_8_LSB 0xE00
#define PHASE3_REG_8_MSB 0xF00


//*****************************************************************
//SEND THE FREQUENCY WORD TO THE DDS CHIP
void SendWordDDS(uint16_t ddsword){
   uint16_t tw;

   SCLK_HIGH;
   FSYNC_HIGH;
   FSYNC_LOW;

   tw=ddsword;	

   if((tw&32768)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
   tw=ddsword;

   if((tw&16384)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&8192)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&4096)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&2048)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&1024)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&512)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&256)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
	tw=ddsword;

   if((tw&128)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&64)==0) SDATA_LOW; else  SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&32)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&16)==0) SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&8)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&4)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&2)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    tw=ddsword;

   if((tw&1)==0)   SDATA_LOW; else SDATA_HIGH;
	SCLK_HIGH; SCLK_LOW; SCLK_HIGH;
    FSYNC_HIGH;             //end 16bit word

}

//*****************************************************************
// converts a frequency in Hz to the 9835 register value

uint32_t ConvertFrequency(uint32_t temp){
   uint32_t c;
   c=(uint32_t)(temp/11.64153218e-3);
   return(c);
}


//*****************************************************************
// sends the 32bit word passed, to the FREQ0 Reg of the 9835

void setTxFreq(uint32_t fv){
  uint16_t word;
  uint32_t t;
  
  fv = ConvertFrequency(fv);

  t=fv&0x000000ff;
  word=(uint16_t)(CMD_WRI8_FREQ_DEFER_REG | FREQ0_REG_8_L_LBS | t);
  SendWordDDS(word);

  t=fv&0x0000ff00;
  word=(uint16_t)(CMD_WRI16_FREQ_REG | FREQ0_REG_8_H_LBS | (t>>=8));
  SendWordDDS(word);

  t=fv&0x00ff0000;
  word=(uint16_t)(CMD_WRI8_FREQ_DEFER_REG | FREQ0_REG_8_L_MSB | (t>>=16));
  SendWordDDS(word);

  t=fv&0xff000000;
  word=(uint16_t)(CMD_WRI16_FREQ_REG | FREQ0_REG_8_H_MSB | (t>>=24));
  SendWordDDS(word);

}


//*****************************************************************
// send the required commands to initialise the 9835

void initTX(void){
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = ( BIT(FSYNC) | BIT(SCLK) | BIT(SDATA) );
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   SendWordDDS(CMD_SET_SLEEP_RST_CLR | SLEEP_HIGH | RESET_HIGH | CLR_HIGH);
   SendWordDDS(CMD_SET_SYNC_AND_SELSRC | SYNC_HIGH | SELSRC_HIGH );

   setTxFreq(500);						 //initial frequency
   SendWordDDS(CMD_WRI8_PHASE_DEFER_REG | PHASE0_REG_8_LSB | 0); //clear phase register
   SendWordDDS(CMD_WRI16_PHASE_REG | PHASE0_REG_8_MSB | 0);

   SendWordDDS(CMD_SELECT_FREQ_REG);
   SendWordDDS(CMD_SELECT_PHASE_REG);

   SendWordDDS(CMD_SET_SLEEP_RST_CLR);

}

