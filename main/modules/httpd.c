#include "globals.h"
#include "httpd.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <sys/param.h>
#include "esp_eth.h"
#include <esp_http_server.h>

#include "messages.h"

static const char *TAG = "HTTPD";

static char* HTML;

/* Our URI handler function to be called during GET / request */
esp_err_t get_handler(httpd_req_t *req)
{   
    char*  buf;
    size_t buf_len;
    char variable[32];
  
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
        } // if (httpd_query_key_value(buf, "command", variable, sizeof(variable)) == ESP_OK) {
        
       if (httpd_query_key_value(buf, "gotoStation", variable, sizeof(variable)) == ESP_OK) {
           ESP_LOGI(TAG, "HTTP GET (goto) Command %s",variable);
           struct AMessage *pxMessage;
           xMessage.ucMessage = GOTO_PRG;
           xMessage.ucNumMessage = atoi(variable);
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
  padding: 15px 32px; \
  text-align: center; \
  text-decoration: none; \
  display: inline-block; \
  font-size: 16px; \
  margin: 4px 2px; \
  cursor: pointer; \
}  \
.button1 {width: 200px;} \
</style> \
</head> \
<body style=\"background-color:powderblue;\"> \
<center> \
<h1>iRadioMini for ESP32</h1> \
\
<form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"Prev\"class=\"button button1\"/> \
<form action=\"\"  method=\"get\"><input name=\"command\" type=\"submit\" value=\"Next\"class=\"button button1\"/> \
<form action=\"\"  method=\"get\"><input name=\"command\" type=\"submit\" value=\"Stop\"class=\"button button1\"/> \
<form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"Play\"class=\"button button1\"/> \
<form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"Vol-\"class=\"button button1\"/> \
<form action=\"\" method=\"get\"><input name=\"command\" type=\"submit\"  value=\"Vol+\"class=\"button button1\"/> \
</form>");


if (!MEDIAPLAYER_ENABLED) {
    // build station list
	strcat(HTML,"<br> <form action=\"\" method=\"GET\"> <select name=\"gotoStation\" id=\"Stations\" onchange=\"this.form.submit()\" STYLE=\"width: 80%\" size=\"25\" > ");

    char itoa_buf[3];
    
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
	
}

/*
strcat(HTML,"<br>  \
<select name=\"Cars\" size=\"25\"> \
    <option value=\"Merceders:&\"> Merceders </option> \
    <option value=\"BMW\"> BMW </option> \
    <option value=\"Jaguar\"> Jaguar </option> \
    <option value=\"Lamborghini\"> Lamborghini </option> \
    <option value=\"Ferrari\"> Ferrari </option> \
    <option value=\"Ford\"> Ford </option>  \
</select>");
*/

strcat(HTML,"</center> \
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
