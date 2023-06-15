#include "globals.h"
#include "httpd.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <sys/param.h>
#include "esp_eth.h"
#include <esp_http_server.h>
#include "sdcard.h"

#include "messages.h"

static const char *TAG = "HTTPD";

static char* HTML;


/* Our URI handler function to be called during GET / request */
esp_err_t get_handler(httpd_req_t *req)
{   
    char*  buf;
    size_t buf_len;
    char variable[32];
    char itoa_buf[5];
  
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
      buf = (char*)malloc(buf_len);
      if(!buf){
        httpd_resp_send_500(req);
        return ESP_FAIL;
      }
      if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
        if (httpd_query_key_value(buf, "command", variable, sizeof(variable)) == ESP_OK) {

            ESP_LOGI(TAG, "HTTP GET Command %s",variable);
            if (!strcmp(variable,"Next")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = NEXT_PRG;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"Prev")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = PREV_PRG;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"Stop")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = STOP;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"Play")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = PLAY;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"Vol%2B")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = VOLUP;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"Vol-")) {
               struct AMessage *pxMessage;
               xMessage.ucMessage = VOLDOWN;
	       pxMessage = &xMessage; 
	       xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

            if (!strcmp(variable,"RESET")) {
               for (int i=0; i<20; i++) {
	          equalizer_gain[i]=-13; // -13db is ESP-ADF default setting
               }          
               buf[0]='\0'; // buf (URL-Anhang) löschen, damit die EQ-default Werte nicht gleich wieder überschrieben werden
               struct AMessage *pxMessage;
               xMessage.ucMessage = UPDEQU;
               pxMessage = &xMessage; 
               xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            }

           if (!strcmp(variable,"SAVEEQ")) {
               saveEQ_SDCard();
           }

        } // if (httpd_query_key_value(buf, "command", variable, sizeof(variable)) == ESP_OK) {
        
       if (httpd_query_key_value(buf, "gotoStation", variable, sizeof(variable)) == ESP_OK) {
           ESP_LOGI(TAG, "HTTP GET (goto) Command %s",variable);
           struct AMessage *pxMessage;
           xMessage.ucMessage = GOTO_PRG;
           xMessage.ucNumMessage = atoi(variable);
	   pxMessage = &xMessage; 
	   xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
            
       }

       // new Equalizer gain settings
       if (httpd_query_key_value(buf, "31Hz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[0] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "62Hz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[1] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "125Hz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[2] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "250Hz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[3] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "500Hz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[4] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "1kHz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[5] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "2kHz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[6] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "4kHz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[7] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "8kHz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[8] = atoi(variable);
       }

       if (httpd_query_key_value(buf, "16kHz", variable, sizeof(variable)) == ESP_OK) {
          equalizer_gain[9] = atoi(variable);
          struct AMessage *pxMessage;
          xMessage.ucMessage = UPDEQU;
          pxMessage = &xMessage; 
          xQueueSend( xPlayerQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
       }


      } //  if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
 
     free(buf);
    } // if (buf_len > 1) {


    /* Send a simple response */
   HTML[0]='\0'; // empty HTML response
   strcat(HTML,"<!DOCTYPE html>\
<html> \
<head> \
<style> \
.button { \
  background-color: #4CAF50; \
  border: none; \
  color: white; \
  padding: 5px; \
  text-align: center; \
  text-decoration: none; \
  display: inline-block; \
  font-size: 16px; \
  margin: 1px; 1px; \
  cursor: pointer; \
  width: 50px; height: 50px; \
}  \
.button_eqn { \
 background-color: #4CAF50; \
  border: none; \
  color: white; \
  padding: 5px; \
  text-align: center; \
  text-decoration: none; \
  display: inline-block; \
  font-size: 16px; \
  margin: 1px; 1px; \
  cursor: pointer; \
  width: 200px; height: 40px; \
} \
input[type=range][orient=vertical] \
{ \
    writing-mode: bt-lr; /* IE */ \
    -webkit-appearance: slider-vertical; /* Chromium */ \
    width: 8px; \
    height: 175px; \
    padding: 0 5px; \
} \
.animated-text { \
  font-family: monospace; \
  overflow: hidden; \
  height:1.1em; \
  word-wrap: break-word; \
  white-space: nowrap; \
  animation: typing 4s steps(100) forwards; \
} \
@keyframes typing { \
   from { \
      width: 0; \
   } \
   to { \
      width: 100ch; \
   } \
} \
</style> \
</head> \
<body style=\"background-color:powderblue;\"> \
<center> ");

// now playing ?
strcat(HTML,"<p class=\"animated-text\"> iRadioMini on ESP32 now playing: <span id=\"nowPlaying\">0</span>");
strcat(HTML," </p> ");

// player controls
strcat(HTML,"<form action=\"\" method=\"get\"> \
<button name=\"command\" type=\"submit\"  value=\"Prev\"class=\"button\">Prev</button> \
<button name=\"command\" type=\"submit\"  value=\"Next\"class=\"button\">Next</button> \
<button name=\"command\" type=\"submit\"  value=\"Stop\"class=\"button\">Stop</button> \
<button name=\"command\" type=\"submit\"  value=\"Play\"class=\"button\">Play</button> \
<button name=\"command\" type=\"submit\"  value=\"Vol-\"class=\"button\">Vol-</button> \
<button name=\"command\" type=\"submit\"  value=\"Vol+\"class=\"button\">Vol+</button> \
</form>");

strcat(HTML,"<br> <form action=\"\" method=\"GET\"> <select name=\"gotoStation\" id=\"Stations\" onchange=\"this.form.submit()\" STYLE=\"width: 65%\" size=\"20\" > ");

if (!MEDIAPLAYER_ENABLED) {
    // build station list
  	for (int i=0;i<channels_in_list;i++) {
  	   strcat(HTML,"<option value=\"");
  	   itoa(i, itoa_buf, 10); 
  	   strcat(HTML,itoa_buf);
  	   strcat(HTML,"\"> ");
  	   strcat(HTML,playlist[i]);
  	   strcat(HTML," </option> ");
	}
	strcat(HTML,"</select></form>");
	
} else {
	// build SDcard mediafile list
        char *url_buf = NULL;
 	for (int i=0;i<sdcard_list_get_url_num(sdcard_list_handle);i++) {
           strcat(HTML,"<option value=\"");
  	   itoa(i, itoa_buf, 10); 
  	   strcat(HTML,itoa_buf);
  	   strcat(HTML,"\"> ");
  	   sdcard_list_choose(sdcard_list_handle, i, &url_buf);
  	   strcat(HTML,url_buf);
  	   strcat(HTML," </option> ");	
	}
	strcat(HTML,"</select></form>");
	sdcard_list_choose(sdcard_list_handle, actual_channel_or_file_ID, &url_buf); // restore pointer to correct file 
} // if (!MEDIAPLAYER_ENABLED) {


// EQUALIZER
strcat(HTML, "<br> \
<form><fieldset  STYLE=\"width: 50%\"><legend>EQUALIZER</legend> \
  <input type=\"range\"  method=\"get\" id=\"31Hz\" name=\"31Hz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[0], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\" > \
  <label for=\"31Hz\">31 Hz</label> \
  <input type=\"range\"  method=\"get\" id=\"62Hz\" name=\"62Hz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[1], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"62Hz\">62 Hz</label> \
  <input type=\"range\"  method=\"get\" id=\"125Hz\" name=\"125Hz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[2], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"125Hz\">125 Hz</label> \
  <input type=\"range\"  method=\"get\" id=\"250Hz\" name=\"250Hz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[3], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"250Hz\">250 Hz</label> \
  <input type=\"range\"  method=\"get\" id=\"500Hz\" name=\"500Hz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[4], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"500Hz\">500 Hz</label> \
  <input type=\"range\"  method=\"get\" id=\"1kHz\" name=\"1kHz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[5], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"1kHz\">1 kHz</label> \
  <input type=\"range\"  method=\"get\" id=\"2kHz\" name=\"2kHz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[6], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\" > \
  <label for=\"2kHz\">2 kHz</label> \
  <input type=\"range\"  method=\"get\" id=\"4kHz\" name=\"4kHz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[7], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"4kHz\">4 kHz</label> \
  <input type=\"range\"  method=\"get\" id=\"8kHz\" name=\"8kHz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[8], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"8kHz\">8 kHz</label> \
  <input type=\"range\"  method=\"get\" id=\"16kHz\" name=\"16kHz\" orient=\"vertical\" \
         min=\"-30\" max=\"10\" value=\"");
itoa(equalizer_gain[9], itoa_buf, 10);
strcat(HTML,itoa_buf); strcat(HTML,"\" step=\"1\" onchange=\"this.form.submit()\"> \
  <label for=\"16kHz\">16 kHz</label> \
  <div> \
  <form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"SAVEEQ\" class=\"button button_eqn\"/> \
  <form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"RESET\" class=\"button button_eqn\"/> \
  </div> \
</fieldset></form>  ");

strcat(HTML,"</center> \
<script> \
setInterval(function() { \
  getData(); \
}, 2000); \
function getData() { \
  var xhttp = new XMLHttpRequest(); \
  xhttp.onreadystatechange = function() { \
    if (this.readyState == 4 && this.status == 200) { \
      document.getElementById(\"nowPlaying\").innerHTML = \
      this.responseText; \
    } \
  }; \
  xhttp.open(\"GET\", \"nowPlaying\", true); \
  xhttp.send(); \
} \
</script> \
</body> \
</html> \
");

    httpd_resp_send(req, HTML, strlen(HTML));
    return ESP_OK;
}

/* Our URI handler function to be called during POST / request */
esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


esp_err_t XMLHttpRequest_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "XMLHttpRequest_handler");
    char resp[512] ="\0"; 
    if (!MEDIAPLAYER_ENABLED) {
      strcat(resp,playlist[actual_channel_or_file_ID]);
    } else {
      //strcat(resp,"SDCARD");
      char *url_buf = NULL;
      sdcard_list_choose(sdcard_list_handle, actual_channel_or_file_ID, &url_buf);
      strcat(resp,url_buf);
    }
    httpd_resp_set_type(req, "text/plain");    
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

/* URI handler structure for GET / */
httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST / */
httpd_uri_t uri_post = {
    .uri      = "/",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};


httpd_uri_t uri_XMLHttpRequest_handler = {
    .uri      = "/nowPlaying",
    .method   = HTTP_GET,
    .handler  = XMLHttpRequest_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    HTML = malloc(8000*sizeof(char));
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        httpd_register_uri_handler(server, &uri_XMLHttpRequest_handler);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
        free(HTML);
    }
}

void httpd(void *pvParameters) { 
  httpd_handle_t hnd = start_webserver();
 
  // Daemonbetrieb
  while (1) {
      ESP_LOGI(TAG, "alive");
      vTaskDelay(1000/portTICK_PERIOD_MS );
   }

  stop_webserver(hnd);
}
