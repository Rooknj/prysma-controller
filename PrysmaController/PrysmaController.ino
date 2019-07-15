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
  PrysmaWifi::setupWifi(PRYSMA_ID);

  // Configure Over the air uploads
  PrysmaOTA::setupOTA(PRYSMA_ID);

  Serial.printf("[INFO]: %s Ready\n", PRYSMA_ID);

  test();

  PrysmaMQTT::setupMQTT(PRYSMA_ID, onMqttMessage);
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

void loop() {
  PrysmaOTA::handleOTA();
  PrysmaMQTT::handleMQTT();
}
