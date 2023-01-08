#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"

#include "mqtt_client.h"

#include "mqtt.h"

static const char *TAG = "mqtt.c";


static void mqtt_event_handler_cb(
        void *handler_args,
        esp_event_base_t base, 
        int32_t event_id, 
        void *event) {

    esp_mqtt_event_handle_t mqtt_event = (esp_mqtt_event_handle_t)event;
    esp_mqtt_client_handle_t client = mqtt_event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            // Build and publish home assistant discovery
            // ESP_LOGI(TAG, "Publish sensor discovery");
        // msg_id = esp_mqtt_client_publish(client, "<topic>", "<data>", <len>, <qos>, <retain>);
            // ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event_id);
            break;
    }
}


void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_PROJECT_MQTT_BROKER_URL,
        .username = CONFIG_PROJECT_MQTT_BROKER_USERNAME,
        .password = CONFIG_PROJECT_MQTT_BROKER_PASSWORD,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, client);
    esp_mqtt_client_start(client);
}
