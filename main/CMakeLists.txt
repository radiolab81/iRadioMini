set(COMPONENT_SRCS "iRadioMini.c" 
		   "./modules/utils.c"
		   "./modules/player.c"
		   "./modules/gpiod.c"
		   "./modules/gpiod_rotary.c"
		   "./modules/sdcard.c"
		   "./modules/httpd.c"
		   
		   "./modules/display/ssd1306OLED/u8g2_esp32_hal.c"
		   "./modules/display/ssd1306OLED/displayd_i2c.c"

		   "./modules/display/HD44780_I2C/displayd_HD44780_i2c.c"

		   "./modules/display/servo/servo.c"
		
		   "./modules/tx/transmitterd.c"
		   "./modules/tx/AD9832/AD9832.c"
		   "./modules/tx/AD9835/AD9835.c"
		   "./modules/tx/AD9850/AD9850.c" )

set(COMPONENT_ADD_INCLUDEDIRS "./modules")
set(EXTRA_COMPONENT_DIRS "./components")

register_component()


