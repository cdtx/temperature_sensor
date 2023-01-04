#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"

#include "am2320.h"


// ESP-01
// Vert: GPIO0 -> SCL
// Jaune: GPIO2 -> SDA

#define I2C_MASTER_SCL_IO           0                /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           2                /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */

static const char *TAG = "main";

void i2c_master_init(i2c_port_t i2c_num) {
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 1000; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    ESP_ERROR_CHECK(i2c_driver_install(i2c_num, I2C_MODE_MASTER));
    ESP_ERROR_CHECK(i2c_param_config(i2c_num, &conf));
}

void i2c_master_delete(i2c_port_t i2c_num) {
    i2c_driver_delete(i2c_num);
}

void task_sensor_read(void *pvParameters) {
    esp_err_t ret = ESP_OK;

    int16_t humidity = 0;
    int16_t temperature = 0;

    // Init esp8266 i2c driver
    i2c_master_init(I2C_MASTER_NUM);
    
    // Init am2320 sensor
    am2320_init(I2C_MASTER_NUM);

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

    // Release driver
    i2c_master_delete(I2C_MASTER_NUM);
}

void app_main()
{
    xTaskCreate(task_sensor_read, "task_sensor_read", 2048, NULL, 10, NULL);
}
