#ifndef _HASS_ENTITIES_H_
#define _HASS_ENTITIES_H_

#define	HASS_DEVICE_TOPIC	"cdtx/Temperature sensor/71920d38-3717-4d34-b7ad-48f77c00de00"

#define	HASS_ENTITY_TEMPERATURE_PUBLISH_TOPIC	"cdtx/Temperature sensor/71920d38-3717-4d34-b7ad-48f77c00de00/temperature"
#define	HASS_ENTITY_TEMPERATURE_DISCOVERY_TOPIC	"homeassistant/sensor/301263a6-c29e-47c7-b4bb-8ee5304b342c/config"
#define	HASS_ENTITY_TEMPERATURE_DISCOVERY_DATA	"{\"name\": \"temperature\", \"unique_id\": \"301263a6-c29e-47c7-b4bb-8ee5304b342c\", \"component\": \"sensor\", \"state_topic\": \"cdtx/Temperature sensor/71920d38-3717-4d34-b7ad-48f77c00de00/temperature\", \"unit_of_measurement\": \"\u00b0C\", \"device\": {\"identifiers\": \"71920d38-3717-4d34-b7ad-48f77c00de00\", \"manufacturer\": \"cdtx\", \"model\": \"Temperature sensor\"}}"

#define	HASS_ENTITY_HUMIDITY_PUBLISH_TOPIC	"cdtx/Temperature sensor/71920d38-3717-4d34-b7ad-48f77c00de00/humidity"
#define	HASS_ENTITY_HUMIDITY_DISCOVERY_TOPIC	"homeassistant/sensor/98ade780-2dde-4afc-aef5-0a6f212fc255/config"
#define	HASS_ENTITY_HUMIDITY_DISCOVERY_DATA	"{\"name\": \"humidity\", \"unique_id\": \"98ade780-2dde-4afc-aef5-0a6f212fc255\", \"component\": \"sensor\", \"state_topic\": \"cdtx/Temperature sensor/71920d38-3717-4d34-b7ad-48f77c00de00/humidity\", \"unit_of_measurement\": \"%\", \"device\": {\"identifiers\": \"71920d38-3717-4d34-b7ad-48f77c00de00\", \"manufacturer\": \"cdtx\", \"model\": \"Temperature sensor\"}}"


#endif
