# philips_hue_controller

ESP32-styrning av Philips Hue via MQTT och Hue API v1.

Koden är skriven så att det ska vara enkelt att bygga ut med nya funktioner eller kommunikationsprotokoll.

## Komma igång

Kräver ESP-IDF. Exportera och bygg:

```
. $HOME/esp/esp-idf/export.sh
idf.py build flash monitor
```

Konfiguration i `config.c` — där sätter du Hue Bridge, WiFi och MQTT.

## Status

Pågående hobbyprojekt.
