#ifndef _WIFI_H_
#define _WIFI_H_

esp_err_t wifi_init(EventGroupHandle_t event_group);
esp_err_t wifi_stop(void);

#endif
