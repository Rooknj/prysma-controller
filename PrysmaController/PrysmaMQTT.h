/*
  PrysmaMQTT.h - Library for connecting Prysma-Controller to the MQTT broker and
  handling messages
*/
#ifndef PrysmaMQTT_h
#define PrysmaMQTT_h

#include <Arduino.h>
#include <ESP8266mDNS.h>   // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // MQTT client library
#include <ArduinoJson.h>

// MQTT topic-name strings
#define MQTT_TOP "prysma"
#define MQTT_CONNECTED "connected"
#define MQTT_EFFECT_LIST "effects"
#define MQTT_STATE "state"
#define MQTT_COMMAND "command"
#define MQTT_CONFIG "config"
#define MQTT_DISCOVERY "discovery"
#define MQTT_DISCOVERY_RESPONSE "hello"
#define MQTT_IDENTIFY "identify"

namespace PrysmaMQTT {

// These need to be extern or else you get a "multiple definition" error
extern char CONNECTED_TOPIC[50];           // for sending connection messages
extern char EFFECT_LIST_TOPIC[50];         // for sending the effect list
extern char STATE_TOPIC[50];               // for sending the state
extern char COMMAND_TOPIC[50];             // for receiving commands
extern char CONFIG_TOPIC[50];              // for sending config info
extern char DISCOVERY_TOPIC[50];           // for sending config info
extern char DISCOVERY_RESPONSE_TOPIC[50];  // for sending config info
extern char IDENTIFY_TOPIC[50];            // for sending config info

void setupMQTT(char* id, char* username, char* password);
void handleMQTT();

void onConnect(void (*callback)());
void onCommand(void (*callback)(char*));
void onIdentify(void (*callback)(char*));
void onDiscovery(void (*callback)(char*));

void sendState();
void sendEffectList();
void sendConfig();
void sendDiscoveryResponse();
}  // namespace PrysmaMQTT

#endif