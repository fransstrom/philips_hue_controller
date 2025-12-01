#include "ir_reciever.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "philips_hue_controller.h"
#include "soc/gpio_num.h"
#include <stdio.h>

static QueueHandle_t ir_queue = NULL;
void handleCode(uint32_t nec_code) {
  char *post_data;
  switch (nec_code) {
  case 0x00FFA25D:
    int ledState = gpio_get_level(LED_PIN);
    printf("LED STATE: %d\n", ledState);
    if (ledState == 1) {
      post_data = "{\"on\":false}";
      printf("turning fucking off\n");
      gpio_set_level(LED_PIN, 0);
      setHueState(post_data);
    } else {
      printf("turning fucking on\n");
      gpio_set_level(LED_PIN, 1);
      post_data = "{\"on\":true}";
      setHueState(post_data);
    }
    break;
  case 0x00FF02FD:
    printf("Turning birghtness up");

    post_data = "{\"bri_inc\":50}";
    setHueState(post_data);
    break;
  case 0x00FF9867:
    printf("Dimming light");
    post_data = "{\"bri_inc\":-50}";
    setHueState(post_data);
    break;
  case 0X00FF906F:
    printf("increasing hue\n");
    post_data = "{\"hue_inc\":6400}";
    setHueState(post_data);
  case 0x00FFE01F:
    printf("decrasing hue\n");

    post_data = "{\"hue_inc\":6400}";
    setHueState(post_data);
    break;
  default:
    printf("CODE NOT SET UP YET\n");
    break;
  }
}
bool rmt_rx_done_callback(rmt_channel_handle_t channel,
                          const rmt_rx_done_event_data_t *edata,
                          void *user_data) {
  BaseType_t high_task_wakeup = pdFALSE;
  // Send notification to queue
  xQueueSendFromISR(ir_queue, edata, &high_task_wakeup);
  return high_task_wakeup == pdTRUE;
}
uint32_t decode_nec(rmt_symbol_word_t *symbols, size_t num_symbols) {
  if (num_symbols < 34)
    return 0;

  uint32_t code = 0;
  for (int i = 1; i < 33; i++) {
    if (symbols[i].duration1 > 1000) {
      code |= (1 << (32 - i));
    }
  }
  return code;
}

void ir_rx_task(void *arg) {
  rmt_channel_handle_t rx_channel = (rmt_channel_handle_t)arg;
  rmt_symbol_word_t raw_symbols[64];
  rmt_rx_done_event_data_t rx_data;

  rmt_receive_config_t receive_config = {
      .signal_range_min_ns = 1250,
      .signal_range_max_ns = 12000000,
  };

  // Start the first receive
  ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols),
                              &receive_config));

  while (1) {
    // Wait for data from queue
    if (xQueueReceive(ir_queue, &rx_data, portMAX_DELAY)) {
      ESP_LOGI(TAG, "IR signal received! Symbols: %zu", rx_data.num_symbols);

      // Print first few symbols
      for (size_t i = 0; i < 10 && i < rx_data.num_symbols; i++) {
        ESP_LOGI(TAG, "Symbol %d: {%d:%dus}, {%d:%dus}", i,
                 raw_symbols[i].level0, raw_symbols[i].duration0,
                 raw_symbols[i].level1, raw_symbols[i].duration1);
      }
      // In ir_rx_task, after receiving:
      uint32_t nec_code = decode_nec(raw_symbols, rx_data.num_symbols);
      if (nec_code != 0) {
        ESP_LOGI(TAG, "NEC Code: 0x%08lX", nec_code);
        handleCode(nec_code);
      }
      // Start receiving again for next signal
      ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols),
                                  &receive_config));
    }
  }
}

// Public API
void ir_receiver_init(gpio_num_t gpio_pin) {
  ir_queue = xQueueCreate(10, sizeof(rmt_rx_done_event_data_t));

  rmt_rx_channel_config_t rx_channel_cfg = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 1000000,
      .mem_block_symbols = 64,
      .gpio_num = gpio_pin,
  };

  rmt_channel_handle_t rx_channel = NULL;
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));

  rmt_rx_event_callbacks_t cbs = {
      .on_recv_done = rmt_rx_done_callback,
  };

  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, NULL));
  ESP_ERROR_CHECK(rmt_enable(rx_channel));

  xTaskCreate(ir_rx_task, "ir_rx_task", 4096, rx_channel, 5, NULL);

  ESP_LOGI(TAG, "Initialized on GPIO %d", gpio_pin);
}

int testBuild() { return 10; }
