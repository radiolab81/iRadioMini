
#ifndef _DISPLAYD_LVGL_SKALE_H_
#define _DISPLAYD_LVGL_SKALE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl.h"
#include "../lvgl_helpers.h"

/*********************
 *      DEFINES
 *********************/

#define LV_TICK_PERIOD_MS     10

static lv_disp_draw_buf_t disp_buf;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief      displayd (lvgl based) task for iRadioMini  
 *
 * @param      1: pointer to call parameters 
 *	    
 * @return     none
 */
void displayd_lvgl_skale(void *pvParameters);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
