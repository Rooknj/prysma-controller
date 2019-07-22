#include "PrysmaMQTT.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>   // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // MQTT client library

using namespace PrysmaMQTT;

// MQTT Variables
// TODO: if MQTT_MAX_PACKET_SIZE is less than 512, display a warning that this
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);
char* MQTT_ID;
char* MQTT_USERNAME;
char* MQTT_PASSWORD;
char connectedMessage[50];
char disconnectedMessage[50];
void (*connectCallback)();
void (*commandCallback)(char*);
void (*discoveryCallback)(char*);
void (*identifyCallback)(char*);

// Header Definitions
char PrysmaMQTT::CONNECTED_TOPIC[50];    // for sending connection messages
char PrysmaMQTT::EFFECT_LIST_TOPIC[50];  // for sending the effect list
char PrysmaMQTT::STATE_TOPIC[50];        // for sending the state
char PrysmaMQTT::COMMAND_TOPIC[50];      // for receiving commands
char PrysmaMQTT::CONFIG_TOPIC[50];       // for sending config info
char PrysmaMQTT::DISCOVERY_TOPIC[50];    // for sending config info
char PrysmaMQTT::DISCOVERY_RESPONSE_TOPIC[50];  // for sending config info
char PrysmaMQTT::IDENTIFY_TOPIC[50];            // for sending config info

typedef struct {
  bool wasFound;
  String hostname;
  IPAddress ip;
  uint16_t port;
} MqttBroker;
long lastQueryAttempt = 0;
MqttBroker findMqttBroker() {
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

void handleMessage(char* topic, byte* payload, unsigned int length) {
  Serial.printf("[INFO]: Message arrived on [%s]\n", topic);

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.println(message);
}

void PrysmaMQTT::setupMQTT(char* id, char* username, char* password) {
  MQTT_ID = id;
  MQTT_USERNAME = username;
  MQTT_PASSWORD = password;

  pubSubClient.setCallback(handleMessage);

  snprintf(CONNECTED_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP,
           MQTT_ID, MQTT_CONNECTED);
  Serial.printf("[INFO]: Connected Topic - %s\n", CONNECTED_TOPIC);
  snprintf(EFFECT_LIST_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP,
           MQTT_ID, MQTT_EFFECT_LIST);
  Serial.printf("[INFO]: Effect List Topic - %s\n", EFFECT_LIST_TOPIC);
  snprintf(STATE_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, MQTT_ID,
           MQTT_STATE);
  Serial.printf("[INFO]: State Topic - %s\n", STATE_TOPIC);
  snprintf(COMMAND_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP,
           MQTT_ID, MQTT_COMMAND);
  Serial.printf("[INFO]: Command Topic - %s\n", COMMAND_TOPIC);
  snprintf(CONFIG_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, MQTT_ID,
           MQTT_CONFIG);
  Serial.printf("[INFO]: Config Topic - %s\n", CONFIG_TOPIC);
  snprintf(DISCOVERY_TOPIC, sizeof(DISCOVERY_TOPIC), "%s/%s", MQTT_TOP,
           MQTT_DISCOVERY);
  Serial.printf("[INFO]: Discovery Topic - %s\n", DISCOVERY_TOPIC);
  snprintf(DISCOVERY_RESPONSE_TOPIC, sizeof(DISCOVERY_RESPONSE_TOPIC),
           "%s/%s/%s", MQTT_TOP, id, MQTT_DISCOVERY_RESPONSE);
  Serial.printf("[INFO]: Discovery Response Topic - %s\n",
                DISCOVERY_RESPONSE_TOPIC);
  snprintf(IDENTIFY_TOPIC, sizeof(IDENTIFY_TOPIC), "%s/%s/%s", MQTT_TOP, id,
           MQTT_IDENTIFY);
  Serial.printf("[INFO]: Identify Topic - %s\n", IDENTIFY_TOPIC);

  // Generate connected/disconnected messages
  StaticJsonDocument<50> doc;
  doc["id"] = MQTT_ID;
  doc["connected"] = true;
  serializeJson(doc, connectedMessage);
  doc["connected"] = false;
  serializeJson(doc, disconnectedMessage);
}

boolean connectToMQTT() {
  // Find and set the mqtt broker
  MqttBroker mqttBroker = findMqttBroker();
  if (!mqttBroker.wasFound) {
    Serial.println("[WARNING]: MQTT Broker Not Found");
    return false;
  }
  pubSubClient.setServer(mqttBroker.ip, mqttBroker.port);

  if (pubSubClient.connect(MQTT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                           CONNECTED_TOPIC, 0, true, disconnectedMessage)) {
    Serial.println("[INFO]: Connected to MQTT broker at " +
                   mqttBroker.hostname + " - " + mqttBroker.ip.toString() +
                   ":" + mqttBroker.port);

    // Subscribe to all relevent topics
    pubSubClient.subscribe(COMMAND_TOPIC);
    Serial.printf("[INFO]: Subscribed to %s\n", COMMAND_TOPIC);
    pubSubClient.subscribe(DISCOVERY_TOPIC);
    Serial.printf("[INFO]: Subscribed to %s\n", DISCOVERY_TOPIC);
    pubSubClient.subscribe(IDENTIFY_TOPIC);
    Serial.printf("[INFO]: Subscribed to %s\n", IDENTIFY_TOPIC);

    // Publish that we are connected;
    pubSubClient.publish(CONNECTED_TOPIC, connectedMessage, true);

    connectCallback();
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

void PrysmaMQTT::onConnect(void (*callback)()) { connectCallback = callback; }
void PrysmaMQTT::onCommand(void (*callback)(char*)) {
  commandCallback = callback;
}
void PrysmaMQTT::onDiscovery(void (*callback)(char*)) {
  discoveryCallback = callback;
}
void PrysmaMQTT::onIdentify(void (*callback)(char*)) {
  identifyCallback = callback;
}