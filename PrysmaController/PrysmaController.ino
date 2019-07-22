/*
    ██████╗ ██████╗ ██╗   ██╗███████╗███╗   ███╗ █████╗
    ██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝████╗ ████║██╔══██╗
    ██████╔╝██████╔╝ ╚████╔╝ ███████╗██╔████╔██║███████║
    ██╔═══╝ ██╔══██╗  ╚██╔╝  ╚════██║██║╚██╔╝██║██╔══██║
    ██║     ██║  ██║   ██║   ███████║██║ ╚═╝ ██║██║  ██║
    ╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝
 */

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "PrysmaConfig.h"
#include "PrysmaMQTT.h";
#include "PrysmaOTA.h";
#include "PrysmaWifi.h";

#define DEBUG true

//*******************************************************
// Global Variables
//*******************************************************
char PRYSMA_ID[19];
byte mac[6];
char connectedMessage[50];
char disconnectedMessage[50];

//*******************************************************
// MQTT Message Handlers
//*******************************************************
// Generate the messages to be send on connect/disconnect from MQTT
void setupConnectedMessages() {
  StaticJsonDocument<50> doc;
  doc["id"] = PRYSMA_ID;
  doc["connected"] = true;
  serializeJson(doc, connectedMessage);
  doc["connected"] = false;
  serializeJson(doc, disconnectedMessage);
}

// Send the state of the light via MQTT
void sendState() { Serial.println("Send State"); }

// Send the list of supported effects via MQTT
void sendEffectList() { Serial.println("Send Effect List"); }

// Send the config of the light via MQTT
void sendConfig() { Serial.println("Send Config"); }

// Respond to a discovery query with the config information of the light
void sendDiscoveryResponse() { Serial.println("Send Discovery Response"); }

// Deal with a message on the command topic
void handleCommand(byte *payload) {
  Serial.println("[INFO]: Handling Command Message");
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payload);
  serializeJsonPretty(doc, Serial);

  sendState();
}

// Deal with a discovery query
void handleDiscovery() {
  Serial.println("[INFO]: Handling Discovery Message");
  sendDiscoveryResponse();
}

// Deal with an identify command
void handleIdentify() { Serial.println("[INFO]: Handling Identify Message"); }

void handleMessage(char *topic, byte *payload, unsigned int length) {
  Serial.printf("[INFO]: Message arrived on <%s>\n", topic);

  // Route the message to the appropriate handler
  if (strcmp(topic, COMMAND_TOPIC) == 0) {
    handleCommand(payload);
  } else if (strcmp(topic, DISCOVERY_TOPIC) == 0) {
    handleDiscovery();
  } else if (strcmp(topic, IDENTIFY_TOPIC) == 0) {
    handleIdentify();
  } else {
    Serial.println(
        "[WARNING]: Incoming message topic did not match any that we are "
        "supposed to be subscribed to");
  }
}

// Handle MQTT connections
void handleConnect() {
  // Subscribe to all relevent topics
  mqttClient.subscribe(COMMAND_TOPIC);
  Serial.printf("[INFO]: Subscribed to %s\n", COMMAND_TOPIC);
  mqttClient.subscribe(DISCOVERY_TOPIC);
  Serial.printf("[INFO]: Subscribed to %s\n", DISCOVERY_TOPIC);
  mqttClient.subscribe(IDENTIFY_TOPIC);
  Serial.printf("[INFO]: Subscribed to %s\n", IDENTIFY_TOPIC);

  // Publish that we are connected;
  mqttClient.publish(CONNECTED_TOPIC, connectedMessage, true);
  Serial.printf("[INFO]: Published %s to <%s>\n", connectedMessage,
                CONNECTED_TOPIC);

  // Publish all current light values over MQTT
  sendState();
  sendEffectList();
  sendConfig();
}

//*******************************************************
// Main Functions
//*******************************************************
void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }

  // init the builtin led on the ESP8266
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Generate this light's unique ID from the mac address
  WiFi.macAddress(mac);
  snprintf(PRYSMA_ID, sizeof(PRYSMA_ID), "Prysma-%02X%02X%02X%02X%02X%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("[INFO]: %s Booting\n", PRYSMA_ID);

  // Connect to WiFi
  Serial.println("--- WiFi Setup ---");
  setupWifi(PRYSMA_ID);

  // Configure Over the air uploads
  Serial.println("--- OTA Setup ---");
  setupOTA(PRYSMA_ID);

  // Read config info from config.json
  Serial.println("--- Config Setup ---");
  setupConfig();

  // Initialize MQTT client and topics
  Serial.println("--- MQTT Setup ---");
  setupConnectedMessages();
  setupMqttTopics(PRYSMA_ID);
  onMqttConnect(handleConnect);
  onMqttMessage(handleMessage);
}

void loop() {
  handleOTA();
  handleMqtt(PRYSMA_ID, config.mqttUsername, config.mqttPassword,
             CONNECTED_TOPIC, 0, true, disconnectedMessage);
}
