#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

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
#define DEEP_SLEEP_TIME_US (CONFIG_PROJECT_SLEEP_TIME * 1e6)

#define TEMPERATURE_MIN (-50 * 10)
#define TEMPERATURE_MAX (100 * 10)
#define HUMIDITY_MIN    (0 * 10)
#define HUMIDITY_MAX    (100 * 10)



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

esp_err_t spiffs_init() {
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf;

    size_t total_bytes, used_bytes;

    // Initialize spiffs
    ESP_LOGI(TAG, "Configuring spiffs");
    conf.base_path = "/spiffs";
    conf.partition_label = NULL;
    conf.max_files = 5;
    conf.format_if_mount_failed = false;

    ret = esp_vfs_spiffs_register(&conf);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "spiffs configuration failed [%d]", ret);
        return ESP_FAIL;
    }

    // Collect spiffs infos
    ret = esp_spiffs_info(NULL, &total_bytes, &used_bytes);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Collect spiffs infos failed [%d]", ret);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SPIFFS infos : total:%d, used:%d", total_bytes, used_bytes);

    return ESP_OK;
}

void i2c_master_init() {
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 1000 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

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

void go_deep_sleep(void) {
    ESP_LOGI(TAG, "Entering deep sleep");
    // Stop MQTT and WiFi before entering deep sleep
    i2c_master_delete();
    mqtt_stop();
    wifi_stop();
    // Enter deep sleep, the device resets on wake-up
    // Radio calibration will not be done after the deep-sleep wakeup. This will lead to weaker current.
    esp_deep_sleep_set_rf_option(2);
    esp_deep_sleep(DEEP_SLEEP_TIME_US);
}

void app_main()
{
    esp_err_t ret;
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

    // Init spiffs
    spiffs_init();
    
    // Init esp8266 i2c driver
    i2c_master_init();

    go_deep_sleep();

    // Init temperature acquisition
    am2320_init(PROJECT_I2C_MASTER_ID);

    // Read sensor
    ret = am2320_read_values(&temperature, &humidity);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "am2320_read_values failed");
        go_deep_sleep();
    }
    // If here; the sensor read succeeds, prepare sending it.
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
        2000 / portTICK_PERIOD_MS   // Give the WiFi 2s to become ready
    );

    if((uxBits & PROJECT_WIFI_ENABLED) != PROJECT_WIFI_ENABLED) {
        ESP_LOGE(TAG, "Unable to connect WiFi");
        go_deep_sleep();
    }

    // Manage MQTT init
    mqtt_init(main_event_group);

    ESP_LOGD(TAG, "Wait for MQTT init");
    uxBits = xEventGroupWaitBits(
        main_event_group,
        PROJECT_MQTT_ENABLED,
        pdFALSE,
        pdTRUE,
        100 / portTICK_PERIOD_MS    // Give mqtt 100ms to be established
    );

    if((uxBits & PROJECT_MQTT_ENABLED) != PROJECT_MQTT_ENABLED) {
        ESP_LOGE(TAG, "Unable to connect MQTT");
        go_deep_sleep();
    }

    ESP_LOGD(TAG, "Wifi and MQTT initialized");

    if((temperature >= TEMPERATURE_MIN) && (temperature <= TEMPERATURE_MAX)) {
        // Temperature
        value_to_string(temperature, value_str, sizeof(value_str));
        ESP_LOGI(TAG, "Temperature value: %s", value_str);
        mqtt_publish_temperature(value_str);
    }
    else {
        ESP_LOGE(TAG, "Temperature out of bounds (%d)", temperature);
    }

    if((humidity >= HUMIDITY_MIN) && (humidity <= HUMIDITY_MAX)) {
        // Humidity
        value_to_string(humidity, value_str, sizeof(value_str));
        ESP_LOGI(TAG, "Humidity value: %s", value_str);
        mqtt_publish_humidity(value_str);
    }
    else {
        ESP_LOGE(TAG, "Humidity out of bounds (%d)", humidity);
    }

    vEventGroupDelete(main_event_group);

    go_deep_sleep();
}

