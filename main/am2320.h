#ifndef _AM2320_H_
#define _AM2320_H_

#define AM2320_I2C_ADDRESS_BASE     0x5C
#define AM2320_I2C_ADDRESS_READ     ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_READ)
#define AM2320_I2C_ADDRESS_WRITE    ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_WRITE)

/**
 * \brief Initializes the am2320 handling functions by giving it the choosen
 * esp i2c number.
 * The number is then stored in a static variable, making the whole
 * library not thread safe
 */
void am2320_init(i2c_port_t i2c_num);

/**
 *  \brief Read humidity value as sent by the sensor
 *  \return 10*humidity value.
 */
esp_err_t am2320_read_humidity(int16_t *value);

/**
 *  \brief Read temperature value as sent by the sensor
 *  \return 10*temperature value.
 */
esp_err_t am2320_read_temperature(int16_t *value);

#endif
