#ifndef _AM2320_H_
#define _AM2320_H_

#define AM2320_I2C_ADDRESS_BASE     0x5C
#define AM2320_I2C_ADDRESS_READ     ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_READ)
#define AM2320_I2C_ADDRESS_WRITE    ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_WRITE)

#define AM2320_NOTHING_CHANGED      (0)
#define AM2320_TEMPERATURE_CHANGED  (1 << 0)
#define AM2320_HUMIDITY_CHANGED     (1 << 1)

/**
 * \brief Initializes the am2320 handling functions by giving it the choosen
 * esp i2c number.
 * The number is then stored in a static variable, making the whole
 * library not thread safe
 */
void am2320_init(i2c_port_t i2c_num);

/**
 *  \brief Read temperature and humidity values as sent by the sensor
 *  \param temperature pointer to int16_t for temperature
 *  \param humidity pointer to int16_t for humidity
 *  \return ESP_OK if ok
 */
esp_err_t am2320_read_values(int16_t *temperature, int16_t *humidity);

/**
 *  \brief Compare given values to those stored inside the component
 *  \return Combination of AM2320_TEMPERATURE_CHANGED, AM2320_HUMIDITY_CHANGED
 */
uint8_t am2320_values_changed(int16_t temperature, int16_t humidity);

/**
 * \brief Save data to the component internal memory
 * \return ESP_OK if ok
 */
esp_err_t am2320_save_values(int16_t temperature, int16_t humidity);

#endif
