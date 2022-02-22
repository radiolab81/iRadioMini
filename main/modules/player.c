#include "player.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "audio_mem.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "esp_decoder.h"

#include "board.h"
#include "esp_audio.h"
#include "globals.h"

#include "messages.h" 

static const char *TAG = "PLAYER";
static bool pipeline_ready = false;

static audio_board_handle_t board_handle;
static audio_event_iface_handle_t evt;

static audio_pipeline_handle_t pipeline;
static audio_element_handle_t http_stream_reader, i2s_stream_writer, esp_decoder;


void create_audioplayer_pipeline(int channel_num) {
    ESP_LOGI(TAG, "[ 2.0 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[ 2.1 ] Create http stream to read data");
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_stream_reader = http_stream_init(&http_cfg);

    ESP_LOGI(TAG, "[ 2.2 ] Create i2s stream to write data to codec chip");
    #ifdef USE_INTERNAL_AUDIODAC
      i2s_stream_cfg_t i2s_cfg = I2S_STREAM_INTERNAL_DAC_CFG_DEFAULT();
    #else
      i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    #endif 	
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[ 2.3 ] Create auto-audio-decoder ");
    audio_decoder_t auto_decode[] = {
        DEFAULT_ESP_MP3_DECODER_CONFIG(),
        DEFAULT_ESP_WAV_DECODER_CONFIG(),
        DEFAULT_ESP_AAC_DECODER_CONFIG(),
        DEFAULT_ESP_M4A_DECODER_CONFIG(),
        DEFAULT_ESP_TS_DECODER_CONFIG(),
    };

    esp_decoder_cfg_t decoder_cfg = DEFAULT_ESP_DECODER_CONFIG();
    esp_decoder = esp_decoder_init(&decoder_cfg, auto_decode, 5);


    ESP_LOGI(TAG, "[ 2.4 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    audio_pipeline_register(pipeline, esp_decoder,        "esp");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    ESP_LOGI(TAG, "[ 2.5 ] Link it together http_stream-->audio_decoder-->i2s_stream-->[codec_chip]");
    const char *link_tag[3] = {"http", "esp", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[ 2.6 ] Set up uri");
    // audio_element_set_uri(http_stream_reader, "http://streams.80s80s.de/wave/mp3-192");
    audio_element_set_uri(http_stream_reader,playlist[channel_num]);

    // Example of using an audio event -- START
    ESP_LOGI(TAG, "[ 2.7 ] Set up event listener");
    
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[ 2.8 ] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[ 2.9 ] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[ 3.0 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);
    pipeline_ready = true;    
}

void terminate_audioplayer_pipeline() {
    // doing the hard way!
    ESP_LOGI(TAG, "[ 4.0 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_unregister(pipeline, http_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, esp_decoder);

    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(http_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(esp_decoder);
}


void player(void *pvParameters) {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "[ 1 ] startup Audio Player");
    board_handle = audio_board_init();

    ESP_LOGI(TAG, "[ 1.1 ] Start audio codec chip");
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START); 
    audio_hal_set_volume(board_handle->audio_hal,40); // default volume 

    ESP_LOGI(TAG, "[ 1.2 ] Start and wait for Wi-Fi network");
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    esp_periph_start(set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);
    

    create_audioplayer_pipeline(0);

    // Deamonbetrieb
    while (1) {
      if(pipeline_ready) {
        audio_event_iface_msg_t msg;
        msg.need_free_data = true;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) esp_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(esp_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from auto-audio-decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            continue;
        }
      } // if(pipeline_ready) {
    }
    // Example of using an audio event -- END

   terminate_audioplayer_pipeline();

}


void switchToChannel(int channel) {
   if (playlist[channel]) {
     pipeline_ready=false;
     terminate_audioplayer_pipeline();
     create_audioplayer_pipeline(channel);
     pipeline_ready=true;
   }
}

void playerControlTask( void * pvParameters )
{
  esp_log_level_set(TAG, ESP_LOG_INFO);

  int actual_channel = 0;
  struct AMessage *rxMsg; 
  struct ADisplaydMessage *txDisplaydMsg;
  
  for( ;; )
  {
    if(xPlayerQueue)
    {
      if( xQueueReceive( xPlayerQueue, &( rxMsg ), ( TickType_t ) 10 )  )
      {
        //ESP_LOGD(TAG, "Message to playerControlTask: %i",rxMsg->ucMessage);
        
        //  Anforderung Kanalumschaltung
        if (rxMsg->ucMessage==NEXT_PRG) {
          if (actual_channel<(channels_in_list-1))
	    actual_channel++;
          else { 
	    actual_channel=0;
	  }	
	    switchToChannel(actual_channel);
        } // if (rxMsg->ucMessage==NEXT_PRG) {

	if (rxMsg->ucMessage==PREV_PRG) {
          if (actual_channel==0)
	     actual_channel=(channels_in_list-1);
          else {
	    actual_channel--;    
	  }
	    switchToChannel(actual_channel);
        } // if (rxMsg->ucMessage==PREV_PRG) {

	if (rxMsg->ucMessage==STOP) {
          if (pipeline_ready)
            audio_pipeline_pause(pipeline);
        } // if (rxMsg->ucMessage==STOP) {

	if (rxMsg->ucMessage==PLAY) {
	  if (pipeline_ready)
            audio_pipeline_resume(pipeline);
        } // if (rxMsg->ucMessage==PLAY) {

	// Anforderung Kanalinfo -> Antwort über xDisplaydQueue
        if (rxMsg->ucMessage==GET_CHANNEL_INFO) {
            xDisplaydMessage.ucMessage = GET_CHANNEL_INFO;
	    xDisplaydMessage.uciChannelNum = actual_channel;
	    xDisplaydMessage.ucURI = playlist[actual_channel];
            
            audio_element_info_t music_info = {0};
            audio_element_getinfo(esp_decoder, &music_info);
  	    xDisplaydMessage.music_info = music_info;

            txDisplaydMsg = &xDisplaydMessage; 
	    xQueueSend( xDisplaydQueue, ( void * ) &txDisplaydMsg, ( TickType_t ) 0 );
         } // if (rxMsg->ucMessage==GET_CHANNEL_NUM) {

     }

     vTaskDelay(10/portTICK_PERIOD_MS );
   }
  }
}



