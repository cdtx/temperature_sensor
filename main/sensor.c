#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
#include "am2320.h"

static const char *TAG = "sensor.c";

void task_sensor_read(void *pvParameters) {
    esp_err_t ret = ESP_OK;

    int16_t humidity = 0;
    int16_t temperature = 0;

    // Init am2320 sensor
    am2320_init(CONFIG_PROJECT_I2C_MASTER_ID);

    while(1) {
        ret = am2320_read_humidity(&humidity);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Read humidity: %d", humidity);
        }
        else {
            ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(ret));
        }
        vTaskDelay(2500 / portTICK_RATE_MS);

        ret = am2320_read_temperature(&temperature);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Read temperature: %d", temperature);
        }
        else {
            ESP_LOGE(TAG, "Error reading humidity: %s", esp_err_to_name(ret));
        }
        vTaskDelay(2500 / portTICK_RATE_MS);
    }
}

void sensor_init(void) {
    xTaskCreate(task_sensor_read, "task_sensor_read", 2048, NULL, 10, NULL);
}

