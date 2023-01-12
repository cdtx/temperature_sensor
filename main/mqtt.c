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

#include "project_config.h"

#include "hass_entities.h"

static const char *TAG = "mqtt.c";

static EventGroupHandle_t main_event_group;
static esp_mqtt_client_handle_t client;

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
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Build and publish home assistant discovery
            ESP_LOGI(TAG, "Publish temperature discovery");
            msg_id = esp_mqtt_client_publish(
                client,
                HASS_ENTITY_TEMPERATURE_DISCOVERY_TOPIC,
                HASS_ENTITY_TEMPERATURE_DISCOVERY_DATA,
                0, 0, 0
            );
            ESP_LOGI(TAG, "Publish humidity discovery");
            msg_id = esp_mqtt_client_publish(
                client,
                HASS_ENTITY_HUMIDITY_DISCOVERY_TOPIC,
                HASS_ENTITY_HUMIDITY_DISCOVERY_DATA,
                0, 0, 0
            );
            ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
            // Declare the MQTT module as enabled
            xEventGroupSetBits(
                main_event_group,
                PROJECT_MQTT_ENABLED
            );
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            // Declare the MQTT module as disabled
            xEventGroupClearBits(
                main_event_group,
                PROJECT_MQTT_ENABLED
            );
            
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event_id);
            break;
    }
}

esp_err_t mqtt_init(EventGroupHandle_t event_group) {
    main_event_group = event_group;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_PROJECT_MQTT_BROKER_URL,
        .username = CONFIG_PROJECT_MQTT_BROKER_USERNAME,
        .password = CONFIG_PROJECT_MQTT_BROKER_PASSWORD,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, client);
    esp_mqtt_client_start(client);
    return ESP_OK;
}

esp_err_t mqtt_stop() {
    xEventGroupClearBits(
        main_event_group,
        PROJECT_MQTT_ENABLED
    );
    return esp_mqtt_client_stop(client);   
}

esp_err_t mqtt_publish_temperature(char *value_str) {
    esp_mqtt_client_publish(
        client,
        HASS_ENTITY_TEMPERATURE_PUBLISH_TOPIC,
        value_str,
        0, 0, 1
    );

    return ESP_OK;
}

esp_err_t mqtt_publish_humidity(char *value_str) {
    esp_mqtt_client_publish(
        client,
        HASS_ENTITY_HUMIDITY_PUBLISH_TOPIC,
        value_str,
        0, 0, 1
    );

    return ESP_OK;
}

