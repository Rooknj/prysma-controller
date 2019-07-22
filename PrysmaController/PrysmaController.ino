/*
    ██████╗ ██████╗ ██╗   ██╗███████╗███╗   ███╗ █████╗
    ██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝████╗ ████║██╔══██╗
    ██████╔╝██████╔╝ ╚████╔╝ ███████╗██╔████╔██║███████║
    ██╔═══╝ ██╔══██╗  ╚██╔╝  ╚════██║██║╚██╔╝██║██╔══██║
    ██║     ██║  ██║   ██║   ███████║██║ ╚═╝ ██║██║  ██║
    ╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝
 */

#include <ESP8266WiFi.h>

#include "PrysmaConfig.h"
#include "PrysmaMQTT.h";
#include "PrysmaOTA.h";
#include "PrysmaWifi.h";

#define DEBUG true

// Prysma Variables
char PRYSMA_ID[19];
byte mac[6];

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
  setupMQTT(PRYSMA_ID, config.mqttUsername, config.mqttPassword);
  onConnect(handleConnect);
}

void onMqttMessage(char *topic, byte *payload, unsigned int length) {
  Serial.printf("[INFO]: Message arrived on [%s]\n", topic);

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.println(message);
}

void handleConnect() {
  Serial.println("Conn123");
  // Publish that we connected
  // client.publish(MQTT_LIGHT_CONNECTED_TOPIC, buffer, true);

  // publish the initial values
  // sendState();
  // sendEffectList();
  // sendConfig(false);
}

void loop() {
  handleOTA();
  handleMQTT();
}
