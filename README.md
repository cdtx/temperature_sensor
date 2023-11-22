# WiFi temperature sensor

- Based on ESP8266 (ESP01)
- Using AM2320 i2c sensor for both temperature and humidity
- Publishing on MQTT
- HomeAssistant compatible (auto sends hass mqtt dicovery)

# Build

## Prepare environment

- With the esp8266 toolchain installed
- With the following declared in any .bashrc

``` bash
    # espressif environments
    function use_esp8266() {
        export PATH="$PATH:$HOME/developpment/espressif/esp8266/xtensa-lx106-elf/bin"
        export IDF_PATH="$HOME/developpment/espressif/esp8266/ESP8266_RTOS_SDK"
        workon esp8266_3
    }
```

``` bash
    $ make
```
