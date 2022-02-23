#include "displayd_lvgl.h"
#include "globals.h"
#include "messages.h" 

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


static const char *TAG = "DISPLAYD_LVGL";
int old_ch = -1;


static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        // Add your btn code here
    }
}

void lvgl_tick(void *pvParameters) {
 while(1)
    {
         lv_task_handler();
         lv_tick_inc(LV_TICK_PERIOD_MS);
         vTaskDelay(LV_TICK_PERIOD_MS/portTICK_PERIOD_MS ); 
    }
}

void displayd_lvgl(void *pvParameters) {
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


   /* Get the current screen  */
   lv_obj_t * scr = lv_disp_get_scr_act(NULL);


   /*Create a widgets on the currently active screen*/
   lv_obj_t * label_prog =  lv_label_create(scr);
   lv_obj_align(label_prog, LV_ALIGN_CENTER, -10, -20);
   lv_label_set_text(label_prog,"P");

   lv_obj_t * label_prog_num =  lv_label_create(scr);
   lv_obj_align(label_prog_num, LV_ALIGN_CENTER, 0, -20);
   
   lv_obj_t * label_url =  lv_label_create(scr);
   lv_label_set_long_mode(label_url, LV_LABEL_LONG_SCROLL_CIRCULAR);
   lv_obj_set_width(label_url,250);
   lv_obj_align(label_url, LV_ALIGN_CENTER, 0, 0);

   lv_obj_t * btn_next = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
   lv_obj_set_pos(btn_next, 100, 170);                    /*Set its position*/
   lv_obj_set_size(btn_next, 60, 40);                     /*Set its size*/
   lv_obj_add_event_cb(btn_next, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/  

   lv_obj_t * label = lv_label_create(btn_next);          /*Add a label to the button*/
   lv_label_set_text(label, LV_SYMBOL_NEXT);              /*Set the labels text*/
   lv_obj_center(label);

   lv_obj_t * btn_prev = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
   lv_obj_set_pos(btn_prev, 20, 170);                     /*Set its position*/
   lv_obj_set_size(btn_prev, 60, 40);                     /*Set its size*/

   lv_obj_t * label2 = lv_label_create(btn_prev);         /*Add a label to the button*/
   lv_label_set_text(label2, LV_SYMBOL_PREV);             /*Set the labels text*/
   lv_obj_center(label2);

   char buffer[10]; 
   
   TaskHandle_t xlvglTickHandle = NULL;
   xTaskCreate( lvgl_tick, "lvgl_tick", 4096, NULL , tskIDLE_PRIORITY, &xlvglTickHandle );
   configASSERT(xlvglTickHandle);

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
 
                  itoa(rxMsg->uciChannelNum,buffer,10);
                  lv_label_set_text(label_prog_num, buffer);
                  lv_label_set_text(label_url, rxMsg->ucURI);     
        
                } // if (old_ch!=rxMsg->uciChannelNum) {
 	       old_ch = rxMsg->uciChannelNum;
 
            }
        }
    
        ESP_LOGI(TAG, "Heartbeat");     
        vTaskDelay(1000/portTICK_PERIOD_MS ); 
    }

  if (xlvglTickHandle)
     vTaskDelete(xlvglTickHandle);

  vTaskDelete(NULL);
}

