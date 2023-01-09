#ifndef _MQTT_H_
#define _MQTT_H_

void mqtt_init(void);
esp_err_t mqtt_publish_temperature(char *value_str);
esp_err_t mqtt_publish_humidity(char *value_str);

#endif
