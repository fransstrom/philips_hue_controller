#include <stdio.h>

#include "config.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "led_controller.h"
#include "pir_sensor.h"
#include "src/ir_reciever.h"
#include "sys/time.h"
#include "time.h"
#include "wifi_connection.h"
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // 13-bit resolution (0-8191)
#define LEDC_FREQUENCY 5000
// In your main initialization code

void time_sync_init(void) {
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_init();

  // Set timezone for Stockholm, Sweden
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  ESP_LOGI("TIME", "Waiting for time synchronization...");

  // Wait for time to be set
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int retry_count = 10;

  while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    time(&now);
    localtime_r(&now, &timeinfo);
  }

  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI("TIME", "Time synchronized: %s", strftime_buf);
}

void app_main(void) {
  printf("GOOD %s", TAG);
  setupLed();
  ir_receiver_init(IR_RX_GPIO);
  pir_sensor_init(PIR_SENSOR_GPIO);
  connect_wifi();
  time_sync_init();
}
