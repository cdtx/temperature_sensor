#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "driver/i2c.h"
#include "sensor.h"
#include "wifi.h"
#include "mqtt.h"

// ESP-01
// Vert: GPIO0 -> SCL
// Jaune: GPIO2 -> SDA

#define I2C_MASTER_SCL_IO           0                /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           2                /*!< gpio number for I2C master data  */

static const char *TAG = "main";

void i2c_master_init() {
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 1000; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(
        i2c_driver_install(
            CONFIG_PROJECT_I2C_MASTER_ID, 
            I2C_MODE_MASTER
        )
    );
    ESP_ERROR_CHECK(
        i2c_param_config(
            CONFIG_PROJECT_I2C_MASTER_ID, 
            &conf
        )
    );
}

void i2c_master_delete() {
    i2c_driver_delete(CONFIG_PROJECT_I2C_MASTER_ID);
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Configure log levels
    esp_log_level_set("*", ESP_LOG_INFO);

    // Init esp8266 i2c driver
    i2c_master_init();
    // Initialize tcpip stack, only once
    tcpip_adapter_init();
    // Initialize event loop, only once
    esp_event_loop_create_default();
    // Initialize nvs, only once
    ESP_ERROR_CHECK(nvs_flash_init());

    // Run temperature acquisition
    sensor_init();

    // Manage WiFi initialisation
    wifi_init();

    // Manage MQTT init
    mqtt_init();
}
