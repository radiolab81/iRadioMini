#ifndef _PLAYER_H_
#define _PLAYER_H_


/**
 * @brief      audioplayer main task. 
 *
 * @param      1: pointer to call parameters 
 *	    
 * @return     none
 */
void player(void *pvParameters);


/**
 * @brief      audioplayer control task - receive msg (next/prev chan...) thru queue 
 *
 * @param      1: pointer to call parameters 
 *
 * @return     none
 */
void playerControlTask(void *pvParameters);


/**
 * @brief      switch to channel in playlist.m3u
 * @param      1: num of ch in playlist 
 *
 * @return     none
 */
void switchToChannel(int channel);

/**
 * @brief      set-up the audio pipeline (HTTP->ESP DECODER->I2S DAC) for playing a channel from playlist
 * @param      1: num of ch in playlist 
 *
 * @return     none
 */
void create_audioplayer_pipeline(int channel_num);


/**
 * @brief      kill the actual audio pipeline
 * @param      none
 *
 * @return     none
 */
void terminate_audioplayer_pipeline();


#endif
