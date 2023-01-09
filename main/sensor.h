#ifndef _SENSOR_H_
#define _SENSOR_H_

void sensor_init(void);
esp_err_t sensor_read(int16_t *temperature, int16_t *humidity);

#endif
