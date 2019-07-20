#include "PrysmaMQTT.h"
#include <Arduino.h>      // Enables use of Arduino specific functions and types
#include <ESP8266mDNS.h>  // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // MQTT client library

using namespace PrysmaMQTT;

// MQTT topic-name strings
#define MQTT_TOP "prysma"
#define MQTT_CONNECTED "connected"
#define MQTT_EFFECT_LIST "effects"
#define MQTT_STATE "state"
#define MQTT_COMMAND "command"
#define MQTT_CONFIG "config"

// MQTT Variables
// TODO: if MQTT_MAX_PACKET_SIZE is less than 512, display a warning that this
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);
char* MQTT_ID;
char* MQTT_USERNAME;
char* MQTT_PASSWORD;

// Header Definitions
char PrysmaMQTT::PRYSMA_CONNECTED_TOPIC[50];  // for sending connection messages
char PrysmaMQTT::PRYSMA_EFFECT_LIST_TOPIC[50];  // for sending the effect list
char PrysmaMQTT::PRYSMA_STATE_TOPIC[50];        // for sending the state
char PrysmaMQTT::PRYSMA_COMMAND_TOPIC[50];      // for receiving commands
char PrysmaMQTT::PRYSMA_CONFIG_TOPIC[50];       // for sending config info

long lastQueryAttempt = 0;
MqttBroker PrysmaMQTT::findMqttBroker() {
  // Find all mqtt service advertisements over MDNS
  int n = MDNS.queryService("mqtt", "tcp");

  // If none were found, return null;
  if (n == 0) {
    Serial.println("[WARNING]: No MQTT services found");
    return {false};
  }

  // Loop through all the services found and pick the best one
  Serial.println("--- MDNS Query Results ---");
  for (int i = 0; i < n; ++i) {
    String SERVICE_NAME = MDNS.hostname(i);
    IPAddress SERVICE_IP = MDNS.IP(i);
    uint16_t SERVICE_PORT = MDNS.port(i);
    Serial.printf("[INFO]: MDNS Result %i:\n", i);
    Serial.println("[INFO]: SERVICE Hostname - " + SERVICE_NAME);
    Serial.println("[INFO]: SERVICE Host IP - " + SERVICE_IP.toString());
    Serial.printf("[INFO]: SERVICE Port - %i\n", SERVICE_PORT);
    Serial.println("------");

    // Services at prysma.local take priority
    if (SERVICE_NAME.indexOf("prysma") >= 0) {
      MqttBroker mqttBroker = {true, SERVICE_NAME, SERVICE_IP, SERVICE_PORT};
      return mqttBroker;
    }
  }

  // If there were no services with the hostname prysma.local, just return the
  // first ip address that came up
  char mqttBrokerIp[16];
  MDNS.IP(0).toString().toCharArray(mqttBrokerIp, sizeof(mqttBrokerIp));
  MqttBroker mqttBroker = {true, MDNS.hostname(0), MDNS.IP(0), MDNS.port(0)};
  return mqttBroker;
}

void PrysmaMQTT::setupMQTT(char* id, char* username, char* password,
                           MQTT_CALLBACK_SIGNATURE) {
  MQTT_ID = id;
  MQTT_USERNAME = username;
  MQTT_PASSWORD = password;
  pubSubClient.setCallback(callback);
  snprintf(PRYSMA_CONNECTED_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, MQTT_ID, MQTT_CONNECTED);
  snprintf(PRYSMA_EFFECT_LIST_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, MQTT_ID, MQTT_EFFECT_LIST);
  snprintf(PRYSMA_STATE_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, MQTT_ID, MQTT_STATE);
  snprintf(PRYSMA_COMMAND_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, MQTT_ID, MQTT_COMMAND);
  snprintf(PRYSMA_CONFIG_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, MQTT_ID, MQTT_CONFIG);
  Serial.printf("[INFO]: Connected Topic - %s\n", PRYSMA_CONNECTED_TOPIC);
  Serial.printf("[INFO]: Effect List Topic - %s\n", PRYSMA_EFFECT_LIST_TOPIC);
  Serial.printf("[INFO]: State Topic - %s\n", PRYSMA_STATE_TOPIC);
  Serial.printf("[INFO]: Command Topic - %s\n", PRYSMA_COMMAND_TOPIC);
  Serial.printf("[INFO]: Config Topic - %s\n", PRYSMA_CONFIG_TOPIC);
}

boolean connectToMQTT() {
  MqttBroker mqttBroker = findMqttBroker();

  if (!mqttBroker.wasFound) {
    Serial.println("[WARNING]: MQTT Broker Not Found");
    return false;
  }

  pubSubClient.setServer(mqttBroker.ip, mqttBroker.port);

  if (pubSubClient.connect(MQTT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("[INFO]: Connected to MQTT broker at " +
                   mqttBroker.hostname + " - " + mqttBroker.ip.toString() +
                   ":" + mqttBroker.port);

    // Publish that we connected
    // client.publish(MQTT_LIGHT_CONNECTED_TOPIC, buffer, true);

    // publish the initial values
    // sendState();
    // sendEffectList();
    // sendConfig(false);

    // Subscribe to all light topics
    pubSubClient.subscribe(PRYSMA_COMMAND_TOPIC);
  }
  return pubSubClient.connected();
}

long lastMqttConnectionAttempt = 0;
void PrysmaMQTT::handleMQTT() {
  // If not connected, attempt to make a connection every 5 seconds
  if (!pubSubClient.connected()) {
    long now = millis();
    if (now - lastMqttConnectionAttempt > 5000) {
      Serial.println("[INFO]: Attempting MQTT connection...");
      lastMqttConnectionAttempt = now;
      // Attempt to reconnect
      if (connectToMQTT()) {
        lastMqttConnectionAttempt = 0;
      } else {
        Serial.print("[WARNING]: Failed MQTT Connection, rc=");
        Serial.println(pubSubClient.state());
        Serial.println("[INFO]: Attempting again in 5 seconds");
      }
    }
  } else {
    pubSubClient.loop();
  }
}