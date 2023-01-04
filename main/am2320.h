#ifndef _AM2320_H_
#define _AM2320_H_

#define AM2320_I2C_ADDRESS_BASE     0x5C
#define AM2320_I2C_ADDRESS_READ     ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_READ)
#define AM2320_I2C_ADDRESS_WRITE    ((AM2320_I2C_ADDRESS_BASE << 1) | I2C_MASTER_WRITE)

void am2320_init(i2c_port_t i2c_num);
esp_err_t am2320_read_humidity(int16_t *value);
esp_err_t am2320_read_temperature(int16_t *value);

#endif
