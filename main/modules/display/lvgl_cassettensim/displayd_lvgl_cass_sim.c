#include "displayd_lvgl_cass_sim.h"

#include "globals.h"
#include "messages.h" 

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "cassette.c"
#include "spule.c"

#if LV_USE_PNG && LV_USE_IMG 

#define FPS 20

static const char *TAG = "DISPLAYD_LVGL_CASS_SIM";
int old_ch = -1;

void lvgl_tick(void *pvParameters) {
 while(1)
    {    
         lv_task_handler();
         lv_tick_inc(LV_TICK_PERIOD_MS);
         vTaskDelay(LV_TICK_PERIOD_MS/portTICK_PERIOD_MS ); 
    }
}


void displayd_lvgl_cass_sim(void *pvParameters) {
   // Messagequeues 
   struct ADisplaydMessage *rxMsg; // -> vom Playerprozess
   struct AMessage *txMsg;         // -> zum Playerprozess
   static uint32_t size_in_px = DISP_BUF_SIZE;
/////////////////////////////////// lib & driver init //////////////////////////////////////////
   lv_init();
   lvgl_driver_init();

   lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
   assert(buf1 != NULL);

   /* Use double buffered when not working with monochrome displays */
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
#else
    static lv_color_t *buf2 = NULL;
#endif


#if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820         \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A    \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D     \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
#endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res  = 320;
    disp_drv.ver_res  = 240;

    disp_drv.flush_cb = disp_driver_flush;

    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */
#ifdef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

/////////////////////////////////// END: lib & driver init //////////////////////////////////////////

   static lv_style_t style;
   lv_style_init(&style);
   lv_style_set_bg_color(&style, lv_color_black());  //lv_color_make(0x00, 0x00, 0x00);


   /* Get the current screen  */
   lv_obj_t * scr = lv_disp_get_scr_act(NULL);
   lv_obj_add_style(scr,&style,LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_clean(lv_scr_act());

   // Hintergrund - hier Spulen
   LV_IMG_DECLARE(spule);
   lv_obj_t * img_links = lv_img_create(scr);
   lv_img_set_src(img_links, &spule);
   lv_obj_align(img_links, LV_ALIGN_CENTER, -66, -10); 

   lv_obj_t * img_rechts = lv_img_create(scr);
   lv_img_set_src(img_rechts, &spule);
   lv_obj_align(img_rechts, LV_ALIGN_CENTER, 66, -10); 

   // Cassette vor(!) der Spule
   LV_IMG_DECLARE(cassette);
   lv_obj_t * img_zeiger = lv_img_create(scr);
   lv_img_set_src(img_zeiger, &cassette);
   lv_obj_align(img_zeiger, LV_ALIGN_CENTER, 0, 0 ); 
 
   // Programm URI unten links
   lv_obj_t * label_url =  lv_label_create(scr);
   lv_label_set_long_mode(label_url, LV_LABEL_LONG_SCROLL_CIRCULAR);
   lv_obj_set_width(label_url,100);
   lv_obj_align(label_url, LV_ALIGN_TOP_MID, 55, 42);

 
   TaskHandle_t xlvglTickHandle = NULL;
   xTaskCreate( lvgl_tick, "lvgl_tick", 4096, NULL , tskIDLE_PRIORITY, &xlvglTickHandle );
   configASSERT(xlvglTickHandle);

   // Daemonbetrieb
   int angle = 0;
   while(1)
    {
       // Kanalinfo von PlayerTask holen
       xMessage.ucMessage = GET_CHANNEL_INFO;
       txMsg = &xMessage; 
       xQueueSend( xPlayerQueue, ( void * ) &txMsg, ( TickType_t ) 0 );
      
       if( xQueueReceive( xDisplaydQueue, &( rxMsg ), ( TickType_t ) 10 )  )
          {  
            if (rxMsg->ucMessage==GET_CHANNEL_INFO) {
		ESP_LOGI(TAG, "actual channel num: %i",rxMsg->iChannelNum); 
   
		// rotation angle resolution 0.1 
                while (angle<1800) { // just 2pi/2
                     lv_img_set_angle(img_links, angle);
	             lv_img_set_angle(img_rechts, angle);	
	             angle+=45;
 		     vTaskDelay((1000/FPS)/portTICK_PERIOD_MS ); 	     
                }
		angle=0;

	        // wurde umgeschaltet ?
                if (old_ch!=rxMsg->iChannelNum) {
                   // URI-Anzeige aktualisieren
                   lv_label_set_text(label_url, rxMsg->ucURI);     
        
                } // if (old_ch!=rxMsg->iChannelNum) {
 	      old_ch = rxMsg->iChannelNum;
            }
        }

       ESP_LOGI(TAG, "Heartbeat");       
    }

  if (xlvglTickHandle)
     vTaskDelete(xlvglTickHandle);

  vTaskDelete(NULL);
}

#endif
