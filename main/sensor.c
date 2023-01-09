#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
#include "am2320.h"

#include "mqtt.h"

static const char *TAG = "sensor.c";

esp_err_t sensor_read(int16_t *temperature, int16_t *humidity) {
    esp_err_t ret = ESP_OK;

    ret = am2320_read_humidity(humidity);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Read humidity: %d", *humidity);
    }
    else {
        ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(ret));
    }
    vTaskDelay(2500 / portTICK_RATE_MS);

    ret = am2320_read_temperature(temperature);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Read temperature: %d", *temperature);
    }
    else {
        ESP_LOGE(TAG, "Error reading humidity: %s", esp_err_to_name(ret));
    }
    return ESP_OK;
}

void sensor_init(void) {
    am2320_init(CONFIG_PROJECT_I2C_MASTER_ID);
}

