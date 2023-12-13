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

#define DISCOVERY_TOPIC_SIZE        100
#define DISCOVERY_TOPIC_DATA_SIZE   500
#define PUBLISH_TOPIC_SIZE          100

static const char *TAG = "mqtt.c";

static EventGroupHandle_t main_event_group;
static esp_mqtt_client_handle_t client;

static void publish_discovery(const char *entity_name) {
    int msg_id;
    char file_name[40];
    FILE *discovery_file_hdl;

    char discovery_topic[DISCOVERY_TOPIC_SIZE];
    char discovery_data[DISCOVERY_TOPIC_DATA_SIZE];

    // Read discovery topic in file
    sprintf(file_name, "/spiffs/%s_DT", entity_name);
    ESP_LOGI(TAG, "discovery topic file name: %s", file_name);
    discovery_file_hdl = fopen(file_name, "r");
    if(discovery_file_hdl == NULL) {
        ESP_LOGE(TAG, "Error while opening file");
        return;
    }
    fgets(discovery_topic, sizeof(discovery_topic), discovery_file_hdl);
    fclose(discovery_file_hdl);
    ESP_LOGI(TAG, "discovery topic: %s", discovery_topic);

    // Read discovery data in file
    sprintf(file_name, "/spiffs/%s_DD", entity_name);
    ESP_LOGI(TAG, "discovery data file name: %s", file_name);
    discovery_file_hdl = fopen(file_name, "r");
    if(discovery_file_hdl == NULL) {
        ESP_LOGE(TAG, "Error while opening file");
        return;
    }
    fgets(discovery_data, sizeof(discovery_data), discovery_file_hdl);
    fclose(discovery_file_hdl);
    ESP_LOGI(TAG, "discovery data: %s (%d)", discovery_data, sizeof(discovery_data));

    ESP_LOGI(TAG, "Publish %s discovery", entity_name);
    msg_id = esp_mqtt_client_publish(
        client,
        discovery_topic,
        discovery_data,
        0, 0, 0
    );
    ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
}

static esp_err_t read_publish_topic(const char *entity_name, char *publish_topic) {
    char file_name[40];
    FILE *file_hdl;

    sprintf(file_name, "/spiffs/%s_PT", entity_name);
    file_hdl = fopen(file_name, "r");
    if(file_hdl == NULL) {
        ESP_LOGE(TAG, "Error opening file [%s]", file_name);
        return ESP_FAIL;
    }

    fgets(publish_topic, PUBLISH_TOPIC_SIZE, file_hdl);
    fclose(file_hdl);

    ESP_LOGI(TAG, "%s publish topic: %s", entity_name, publish_topic);

    return ESP_OK;
}

static void mqtt_event_handler_cb(
        void *handler_args,
        esp_event_base_t base, 
        int32_t event_id, 
        void *event) {

    esp_mqtt_event_handle_t mqtt_event = (esp_mqtt_event_handle_t)event;
    esp_mqtt_client_handle_t client = mqtt_event->client;
    // your_context_t *context = event->context;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Build and publish home assistant discovery
            publish_discovery("HUMIDITY");
            publish_discovery("TEMPERATURE");

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

static esp_err_t mqtt_publish_value(const char *entity_name, char *value_str) {
    esp_err_t ret;
    char publish_topic[PUBLISH_TOPIC_SIZE];

    ret = read_publish_topic(entity_name, publish_topic);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Unable to read %s discovery topic", entity_name);
        return ESP_FAIL;
    }

    // Actually publish
    esp_mqtt_client_publish(
        client,
        publish_topic,
        value_str,
        0, 0, 1
    );

    return ESP_OK;
}

esp_err_t mqtt_publish_temperature(char *value_str) {
    return mqtt_publish_value("TEMPERATURE", value_str);
}

esp_err_t mqtt_publish_humidity(char *value_str) {
    return mqtt_publish_value("HUMIDITY", value_str);
}

