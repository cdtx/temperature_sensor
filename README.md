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

# spiffs

The sensor compute and maintain its home assistant entity uids in spiffs files.

Those file live in a dedicated partition (see partition.csv)

A first image of this partition have to be built and loaded

## Create content

All the sensors have template.json in common.

This file is the input for pymqtt\_hass\_resolv

pymqtt\_hass\_resolv shall be called once to generate a config.json with items resolved, for each new device.

``` bash
$ pymqtt\_hass\_resolv <>/template.json json > config.json
```

Then, another time to generate the spiffs folder

``` bash
    $ mkdir out
    $ cd out
    $ pymqtt\_hass\_resolv ../config.json fs
```


## Build image

``` bash
    # Build a spiffs image with size 0x4000 from thespiffs folder
    python <>/spiffsgen.py 0x4000 spiffs spiffs.bin
```

## Load image

``` bash
    # Load the spiffs image
    python <>/esptool.py --chip esp8266 --port /dev/ttyACM0 write_flash -z 0xfc000 spiffs.bin
```
