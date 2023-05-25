#ifndef _MSG_H_
#define _MSG_H_

#include "audio_element.h"

#define NEXT_PRG 10
#define PREV_PRG 11
#define STOP 	 12
#define PLAY     13
#define GOTO_PRG 14
#define UPDEQU   15

#define VOLUP 	 20
#define VOLDOWN  21

#define GET_CHANNEL_INFO 	30

#define ENABLE_MEDIAPLAYER 40
#define ENABLE_INTERNETRADIO 41

struct AMessage
{
    char ucMessage;
    //char ucData[255];
    int  ucNumMessage;

} xMessage;

struct ADisplaydMessage
{
    char ucMessage;
    int  iChannelNum;
    char* ucURI;
    audio_element_info_t music_info;

} xDisplaydMessage;

QueueHandle_t xPlayerQueue;
QueueHandle_t xDisplaydQueue;


struct ATransmitterdMessage
{
    char ucMessage;
    uint32_t new_freq;

} xTransmitterdMessage;

#define SET_FREQ 50
QueueHandle_t xTransmitterdQueue;


#endif
