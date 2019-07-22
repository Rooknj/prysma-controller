/*
  PrysmaMqtt.h - Library for connecting Prysma-Controller to the Mqtt broker and
  handling messages
*/
#ifndef PrysmaMqtt_h
#define PrysmaMqtt_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>   // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // Mqtt client library

// Mqtt topic-name strings
#define MQTT_TOP "prysma"
#define MQTT_CONNECTED "connected"
#define MQTT_EFFECT_LIST "effects"
#define MQTT_STATE "state"
#define MQTT_COMMAND "command"
#define MQTT_CONFIG "config"
#define MQTT_DISCOVERY "discovery"
#define MQTT_DISCOVERY_RESPONSE "hello"
#define MQTT_IDENTIFY "identify"

// These need to be extern or else you get a "multiple definition" error
extern char CONNECTED_TOPIC[50];           // for sending connection messages
extern char EFFECT_LIST_TOPIC[50];         // for sending the effect list
extern char STATE_TOPIC[50];               // for sending the state
extern char COMMAND_TOPIC[50];             // for receiving commands
extern char CONFIG_TOPIC[50];              // for sending config info
extern char DISCOVERY_TOPIC[50];           // for receiving discovery queries
extern char DISCOVERY_RESPONSE_TOPIC[50];  // for sending discovery responses
extern char IDENTIFY_TOPIC[50];            // for receiving identify commands

extern PubSubClient mqttClient;

void setupMqttTopics(char* id);

void handleMqtt(const char* id, const char* user, const char* pass,
                          const char* willTopic, uint8_t willQos,
                          boolean willRetain, const char* willMessage);

void onMqttMessage(MQTT_CALLBACK_SIGNATURE);

void onMqttConnect(void (*callback)());

#endif