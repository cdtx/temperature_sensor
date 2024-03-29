#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"

#include "am2320.h"

#define AM2320_READ_DATA_INDEX      0x00
#define AM2320_STORE_DATA_INDEX     0x10

static const char *TAG = "am2320";
static i2c_port_t i2c_master_num;

static void am2320_wakeup() {
    // Send address
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 
        AM2320_I2C_ADDRESS_WRITE, 
        I2C_MASTER_NACK
    );
    // No error check, sensor is waking up, maybe result is unpredictable
    i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    // Wait 900us until sensor wakes up
    // vTaskDelay(1);
    cmd = i2c_cmd_link_create();
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
}

static uint16_t compute_crc(uint8_t *ptr, uint8_t len) {
    uint16_t crc =0xFFFF;
    uint8_t i;

    while(len--) {
        crc ^= *ptr++;
        for(i=0 ; i<8 ; i++) {
            if(crc & 0x01) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else {
                crc >>= 1;
            }
        }
    }
    ESP_LOGD(TAG, "Computed crc: %.4x", crc);
    return crc;
}

void am2320_init(i2c_port_t i2c_num) {
    i2c_master_num = i2c_num;
}

static esp_err_t am2320_read_4_bytes(uint8_t address, uint8_t *data_read) {
    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd;
    uint8_t data_write[4] = {0x00};
    uint16_t crc, crc_compute;

    am2320_wakeup();

    // Set data to write
    data_write[0] = AM2320_I2C_ADDRESS_WRITE;   // Device Address
    data_write[1] = 0x03;                       // Read
    data_write[2] = address;                    // Index to read at
    data_write[3] = 0x04;                       // Size to read

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write(cmd, data_write, 4, I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    ret |= i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 
        AM2320_I2C_ADDRESS_READ, 
        I2C_MASTER_ACK
    );
    i2c_master_read(cmd, data_read, 8, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret |= i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    i2c_cmd_link_delete(cmd);

    ESP_LOGD(TAG, "Read data: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 
            data_read[0],
            data_read[1],
            data_read[2],
            data_read[3],
            data_read[4],
            data_read[5],
            data_read[6],
            data_read[7]
    );

    // Check CRC (low byte first in frame)
    crc = (data_read[7]<<8) + (data_read[6]);
    crc_compute = compute_crc(data_read, 6);
    if(crc != crc_compute) {
        ESP_LOGE(TAG, "CR check failed [read:%.4x] [computed:%.4x]", crc, crc_compute);
        return ESP_FAIL;
    }

    return ret;
}

esp_err_t am2320_read_values(int16_t *p_temperature, int16_t *p_humidity) {
    esp_err_t ret;
    uint8_t data_read[8] = {0,0,0,0,0,0,0,0};

    ret = am2320_read_4_bytes(AM2320_READ_DATA_INDEX, data_read);

    // Leave p_value unchanged if an error occurs
    if (ret == ESP_OK) {
        *p_humidity = (data_read[2]<<8) + (data_read[3]);
        *p_temperature = (data_read[4]<<8) + (data_read[5]);
    }

    return ret;
}

uint8_t am2320_values_changed(int16_t temperature, int16_t humidity) {
    /* Read the previous values */
    uint8_t ret = AM2320_NOTHING_CHANGED;
    esp_err_t res;
    uint8_t data_read[8] = {0,0,0,0,0,0,0,0};
    int16_t temperature_saved;
    int16_t humidity_saved;

    res = am2320_read_4_bytes(AM2320_STORE_DATA_INDEX, data_read);
    if(res != ESP_OK) {
        ESP_LOGE(TAG, "Error reading stored values");
        return AM2320_HUMIDITY_CHANGED | AM2320_TEMPERATURE_CHANGED;
    }

    humidity_saved = (data_read[2]<<8) + (data_read[3]);
    temperature_saved = (data_read[4]<<8) + (data_read[5]);
    ESP_LOGI(TAG, "Read stored values: temperature:%d, humidity:%d", temperature_saved, humidity_saved);

    if(temperature != temperature_saved) {
        ret |= AM2320_TEMPERATURE_CHANGED;
    }
       
    if(humidity != humidity_saved) {
        ret |= AM2320_HUMIDITY_CHANGED;
    }
    return ret;
}

esp_err_t am2320_save_values(int16_t temperature, int16_t humidity) {
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t data_write[10];
    uint8_t data_read[5];
    uint16_t crc;

    am2320_wakeup();

    data_write[0] = AM2320_I2C_ADDRESS_WRITE;
    data_write[1] = 0x10;                       // Write instruction
    data_write[2] = AM2320_STORE_DATA_INDEX;    // Starting index
    data_write[3] = 4;                          // Size
    data_write[4] = (uint8_t)((humidity & 0xFF00) >> 8);
    data_write[5] = (uint8_t)((humidity & 0x00FF) >> 0);
    data_write[6] = (uint8_t)((temperature & 0xFF00) >> 8);
    data_write[7] = (uint8_t)((temperature & 0x00FF) >> 0);

    crc = compute_crc(&data_write[1], 7);       // Compute CRC (address not included)
    data_write[8] = (uint8_t)((crc & 0x00FF) >> 0);
    data_write[9] = (uint8_t)((crc & 0xFF00) >> 8);

    ESP_LOGD(TAG, "Read to write: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 
            data_write[0],
            data_write[1],
            data_write[2],
            data_write[3],
            data_write[4],
            data_write[5],
            data_write[6],
            data_write[7],
            data_write[8],
            data_write[9]
    );

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write(cmd, data_write, 10, I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    ret |= i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    /* Read response from the sensor */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 
        AM2320_I2C_ADDRESS_READ, 
        I2C_MASTER_ACK
    );
    i2c_master_read(cmd, data_read, 5, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret |= i2c_master_cmd_begin(i2c_master_num, cmd, 1000 / portTICK_RATE_MS);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    i2c_cmd_link_delete(cmd);

    ESP_LOGD(TAG, "Read data: 0x%x 0x%x 0x%x 0x%x", 
            data_read[0],
            data_read[1],
            data_read[2],
            data_read[3]
    );

    /* Nothing done with this result so far... */

    return ESP_OK;
}
