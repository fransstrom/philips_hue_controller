# philips_hue_controller

ESP32-based Philips Hue control over MQTT using the Hue API v1.

The code is structured to make it easy to add new functionality or communication protocols.

## Getting started

You'll need the ESP-IDF:

```
. $HOME/esp/esp-idf/export.sh
idf.py menuconfig    # WiFi only
idf.py build flash monitor
```

Create a `config.c` based on `config.h` with your Hue Bridge, MQTT and GPIO settings.

## Status

Ongoing hobby project.
