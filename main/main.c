#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "driver/i2c.h"

#include "am2320.h"
#include "wifi.h"
#include "mqtt.h"

#include "project_config.h"

// ESP-01
// Vert: GPIO0 -> SCL
// Jaune: GPIO2 -> SDA

#define I2C_MASTER_SCL_IO           0                /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           2                /*!< gpio number for I2C master data  */

static const char *TAG = "main";

static EventGroupHandle_t main_event_group;

/**
 *  \brief Create signed decimal string from int16_t
 *  MSB is the value sign
 *  The rest is the actual value * 10
 */
void value_to_string(int16_t value, char *out, int out_size) {
    bool negative = value & 0x8000;
    snprintf(out, out_size, "%s%d.%.1d",
       negative ? "-":"", 
       (value & 0x7FFF)/10,
       (value & 0x7FFF) % 10
    );
}

void i2c_master_init() {
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 1000; // 1000 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(
        i2c_driver_install(
            PROJECT_I2C_MASTER_ID, 
            I2C_MODE_MASTER
        )
    );
    ESP_ERROR_CHECK(
        i2c_param_config(
            PROJECT_I2C_MASTER_ID, 
            &conf
        )
    );
}

void i2c_master_delete() {
    i2c_driver_delete(PROJECT_I2C_MASTER_ID);
}

void app_main()
{
    int16_t temperature, humidity;
    char value_str[10];
    EventBits_t uxBits;

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Configure log levels
    esp_log_level_set("*", ESP_LOG_INFO);

    // Initialize the statusGroup for this main
    main_event_group = xEventGroupCreate();

    // Init esp8266 i2c driver
    i2c_master_init();
    // Initialize tcpip stack, only once
    tcpip_adapter_init();
    // Initialize event loop, only once
    esp_event_loop_create_default();
    // Initialize nvs, only once
    ESP_ERROR_CHECK(nvs_flash_init());

    // Manage WiFi initialisation
    wifi_init(main_event_group);

    ESP_LOGI(TAG, "Wait for WiFi to be ready");
    uxBits = xEventGroupWaitBits(
        main_event_group,
        PROJECT_WIFI_ENABLED,
        pdFALSE,
        pdTRUE,
        -1
    );
    // Manage MQTT init
    mqtt_init(main_event_group);

    // Run temperature acquisition
    am2320_init(PROJECT_I2C_MASTER_ID);

    while(1) {
        ESP_LOGD(TAG, "Wait for WiFi and MQTT init");
        uxBits = xEventGroupWaitBits(
            main_event_group,
            PROJECT_WIFI_ENABLED | PROJECT_MQTT_ENABLED,
            pdFALSE,
            pdTRUE,
            -1
        );
        ESP_LOGD(TAG, "Wifi and MQTT initialized");
        am2320_read_values(&temperature, &humidity);

        // Temperature
        value_to_string(temperature, value_str, sizeof(value_str));
        ESP_LOGI(TAG, "Temperature value: %s", value_str);
        mqtt_publish_temperature(value_str);

        // Humidity
        value_to_string(humidity, value_str, sizeof(value_str));
        ESP_LOGI(TAG, "Humidity value: %s", value_str);
        mqtt_publish_humidity(value_str);

        vTaskDelay(2500 / portTICK_RATE_MS);
    }

    i2c_master_delete();
    vEventGroupDelete(main_event_group);
}

