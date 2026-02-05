#include "pir_sensor.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "freertos/FreeRTOS.h"
#include "philips_hue_controller.h"
#include "soc/gpio_num.h"
#include <time.h>
// static void IRAM_ATTR motion_isr_handler(void *arg) {
//   // Handle motion detection here (e.g., set a flag or send to queue)
//   ESP_DRAM_LOGI("PIR", "Motion Detected!");
// }
//
// static void motion_detection_task(void *args) {
//   while (1) {
//     ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     ESP_LOGI(TAG_PIR, "Motion Detected!");
//     // Handle motion detection here
//   }
// }
//
// void pir_sensor_init(gpio_num_t gpio) {
//   gpio_config_t pir_conf = {
//       .pin_bit_mask = (1ULL << gpio),
//       .mode = GPIO_MODE_INPUT,
//       .pull_up_en = GPIO_PULLUP_DISABLE,
//       .pull_down_en = GPIO_PULLDOWN_ENABLE,
//       .intr_type = GPIO_INTR_POSEDGE,
//   };
//
//   gpio_config(&pir_conf);
//   ESP_LOGI(TAG, "GPIO initialized (PIR: GPIO%d, LED: GPIO%d)", gpio);
//   xTaskCreate(motion_detection_task, "motion_task", 4096, NULL, 5, NULL);
//   gpio_install_isr_service(0);
//   gpio_isr_handler_add(gpio, motion_isr_handler, NULL);
// }

static TaskHandle_t motion_task_handle = NULL;

static bool is_active_hours(void) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  int hour = timeinfo.tm_hour;

  // Active from 23:00 to 09:00 (11 PM to 9 AM)
  return (hour >= 22 || hour < 8);
}

static void IRAM_ATTR motion_isr_handler(void *arg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(motion_task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void motion_detection_task(void *args) {
  // This is for checking state with GPIO_INTR_ANYEDGE
  // while (1) {
  //   gpio_num_t gpio = (gpio_num_t)(int)args;
  //   const char *post_data;
  //   ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  //   int current_state = gpio_get_level(gpio);
  //   if (currentstate == 1) {
  //     ESP_LOGI(TAG_PIR, "Motion Detected!");
  //     post_data = "{\"on\":true}";
  //     setHueState(HUE_ENDPOINT_HALL, post_data);
  //   } else {
  //     ESP_LOGI(TAG_PIR, "Motion Ended!");
  //     post_data = "{\"on\":false}";
  //     setHueState(HUE_ENDPOINT_HALL, post_data);
  //   }
  //   // setHueState(HUE_ENDPOINT_HALL, const char *post_data)
  //   // Handle motion detection here
  // }
  gpio_num_t gpio = (gpio_num_t)(int)args;
  const char *post_data;
  bool lights_on = false;
  TickType_t last_motion_time = 0;
  const TickType_t timeout = pdMS_TO_TICKS(1 * 60 * 1000); // 1 minutes

  while (1) {

    // Log current time on startup
    // time_t now;
    // struct tm timeinfo;
    // time(&now);
    // localtime_r(&now, &timeinfo);
    // char strftime_buf[64];
    // strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S %Z",
    //          &timeinfo);
    // ESP_LOGI(TAG_PIR, "Task started. Current time: %s",
    //          strftime_buf); // Just checking if time is correct
    //
    if (is_active_hours()) {

      // Wait for motion interrupt OR timeout
      uint32_t notification =
          ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)); // Check every second

      if (notification > 0) {
        // Motion detected
        int current_state = gpio_get_level(gpio);

        if (current_state == 1) {
          ESP_LOGI(TAG_PIR, "Motion Detected!");
          last_motion_time = xTaskGetTickCount();

          // Turn on lights if not already on
          if (!lights_on) {
            post_data = "{\"on\":true}";
            setHueState(HUE_ENDPOINT_HALL, post_data);
            lights_on = true;
          }
        }
      }

      // Check if timeout has elapsed since last motion
      if (lights_on && (xTaskGetTickCount() - last_motion_time) >= timeout) {
        ESP_LOGI(TAG_PIR, "No motion for 1 minutes - turning off lights");
        post_data = "{\"on\":false}";
        setHueState(HUE_ENDPOINT_HALL, post_data);
        lights_on = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(60000)); // Check every minute when inactive
  }
}

void pir_sensor_init(gpio_num_t gpio) {
  gpio_config_t pir_conf = {
      .pin_bit_mask = (1ULL << gpio),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_ANYEDGE, // ← THIS WAS YOUR PROBLEM!
  };
  gpio_config(&pir_conf);

  ESP_LOGI(TAG_PIR, "PIR sensor initialized on GPIO%d", gpio);

  // Create task BEFORE installing ISR (so motion_task_handle is valid)
  xTaskCreate(motion_detection_task, "motion_task", 4096, (void *)(int)gpio, 5,
              &motion_task_handle);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(gpio, motion_isr_handler, NULL);
}
