#include "philips_hue_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include <sys/param.h>

#include "../config.h"
#include "cJSON.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HTTP_OUTPUT_BUFFER 2048
// HTTP response structure
typedef struct {
  char *buf;  // Response buffer
  size_t cap; // Buffer capacity
  size_t len; // Current length of data
} http_resp_t;

void log_resp(http_resp_t *resp) {
  // Create a JSON object
  cJSON *root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "len", resp->len);
  cJSON_AddNumberToObject(root, "cap", resp->cap);
  cJSON_AddStringToObject(root, "data", resp->buf);

  char *out = cJSON_Print(root); // Pretty-print (indented)
  if (out) {
    ESP_LOGI(TAG, "HTTP Response JSON: %s", out);
    free(out); // must free the string
  }

  cJSON_Delete(root);
}

void setHueState(int state) {
  esp_log_level_set("*", ESP_LOG_DEBUG);

  http_resp_t *resp = calloc(1, sizeof(*resp));

  if (!resp) {
    ESP_LOGE(TAG, "Out of memory allocating response struct");
    return;
  }

  resp->cap = MAX_HTTP_OUTPUT_BUFFER;
  resp->buf = malloc(resp->cap + 1); // <-- +1 for terminating NUL
  // is this removing the fin reponse causeing the warning that size is not
  // matching?
  resp->buf[0] = '\0';

  if (!resp->buf) {
    ESP_LOGE(TAG, "Out of memory allocating response buffer");
    free(resp);
    return;
  }

  esp_http_client_config_t config = {
      .url = HUE_ENDPOINT_GROUP,
      // .event_handler = _http_event_handler,
      .user_data = resp,
      .disable_auto_redirect = true,
      .keep_alive_enable = true,
      .timeout_ms = 20000,
      .tls_version = false,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    free(resp->buf);
    free(resp);
    return;
  }

  char *post_data = (state == 1) ? "{\"on\":true}" : "{\"on\":false}";
  int datalen = strlen(post_data);
  esp_http_client_set_url(client, HUE_ENDPOINT_GROUP);
  esp_http_client_set_method(client, HTTP_METHOD_PUT);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, post_data, datalen);

  // According to GPT hue bridge is weird with esp.... So incomplete data alwas
  // return error.
  esp_err_t err = esp_http_client_perform(client);

  // Accept incomplete data if there is something usable
  if (err == ESP_OK || (err == ESP_ERR_HTTP_INCOMPLETE_DATA && resp->len > 0)) {
    ESP_LOGI(TAG, "Response: %s", resp->buf);
  } else {
    log_resp(resp); // dump whatever we got (could be empty)
    ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);

  // ägaren ska frigöra (din event_handler gör inte free när user_data != NULL)
  free(resp->buf);
  free(resp);
}
