#ifndef CONFIG_H
#define CONFIG_H
// Create the c file yourself
#include "soc/gpio_num.h"
extern const int IR_RESOLUTION_HZ;
extern const gpio_num_t LED_PIN;
extern const gpio_num_t IR_RX_GPIO;

extern const char *TAG;
#endif // !CONFIG_H
