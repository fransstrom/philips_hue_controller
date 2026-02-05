#ifndef CONFIG_H
#define CONFIG_H
// Create the c file yourself
#include "soc/gpio_num.h"
extern const int IR_RESOLUTION_HZ;
extern const gpio_num_t LED_PIN;
extern const gpio_num_t IR_RX_GPIO;
extern const gpio_num_t PIR_SENSOR_GPIO;

extern const char *TAG;
extern const char *TAG_PIR;

extern const char WIFI_SSID[];
extern const char WIFI_PASS[];
extern const char HUE_ENDPOINT[];
extern const char HUE_ENDPOINT_GROUP_5[];

// To be implemented
extern const char HUE_ENDPOINT_ALL[];
// To be setup with night motion sensor
extern const char HUE_ENDPOINT_HALL[];
#endif // !CONFIG_H
