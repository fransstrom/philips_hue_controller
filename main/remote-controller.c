#include <stdio.h>

#include "config.h"
#include "led_controller.h"
#include "src/ir_reciever.h"
#include "wifi_connection.h"
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // 13-bit resolution (0-8191)
#define LEDC_FREQUENCY 5000

void app_main(void) {
  printf("GOOD %s", TAG);
  setupLed();
  ir_receiver_init(IR_RX_GPIO);
  // connect_wifi();
}
