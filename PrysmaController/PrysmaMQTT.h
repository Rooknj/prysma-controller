/*
  PrysmaMQTT.h - Library for connecting Prysma-Controller to the MQTT broker and
  handling messages
*/
#ifndef PrysmaMQTT_h
#define PrysmaMQTT_h

#include <Arduino.h>
#include <ESP8266mDNS.h>   // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // MQTT client library

namespace PrysmaMQTT {

typedef struct {
  bool wasFound;
  String hostname;
  IPAddress ip;
  uint16_t port;
} MqttBroker;

// These need to be extern or else you get a "multiple definition" error
extern char PRYSMA_CONNECTED_TOPIC[50];    // for sending connection messages
extern char PRYSMA_EFFECT_LIST_TOPIC[50];  // for sending the effect list
extern char PRYSMA_STATE_TOPIC[50];        // for sending the state
extern char PRYSMA_COMMAND_TOPIC[50];      // for receiving commands
extern char PRYSMA_CONFIG_TOPIC[50];       // for sending config info

MqttBroker findMqttBroker();

void setupMQTT(char* id, MQTT_CALLBACK_SIGNATURE);

boolean connectToMQTT();

void handleMQTT();

}  // namespace PrysmaMQTT

#endif