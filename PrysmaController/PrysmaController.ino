/*
    ██████╗ ██████╗ ██╗   ██╗███████╗███╗   ███╗ █████╗
    ██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝████╗ ████║██╔══██╗
    ██████╔╝██████╔╝ ╚████╔╝ ███████╗██╔████╔██║███████║
    ██╔═══╝ ██╔══██╗  ╚██╔╝  ╚════██║██║╚██╔╝██║██╔══██║
    ██║     ██║  ██║   ██║   ███████║██║ ╚═╝ ██║██║  ██║
    ╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // MQTT client library

#include "PrysmaConfig.h"
#include "PrysmaMDNS.h";
#include "PrysmaOTA.h";
#include "PrysmaWifi.h";

#define DEBUG true

// MQTT topic-name strings
#define MQTT_TOP "prysma"
#define MQTT_CONNECTED "connected"
#define MQTT_EFFECT_LIST "effects"
#define MQTT_STATE "state"
#define MQTT_COMMAND "command"
#define MQTT_CONFIG "config"

// Prysma Variables
char PRYSMA_ID[19];
byte mac[6];

// MQTT Variables
// TODO: if MQTT_MAX_PACKET_SIZE is less than 512, display a warning that this
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);
char PRYSMA_CONNECTED_TOPIC[50];    // for sending connection messages
char PRYSMA_EFFECT_LIST_TOPIC[50];  // for sending the effect list
char PRYSMA_STATE_TOPIC[50];        // for sending the state
char PRYSMA_COMMAND_TOPIC[50];      // for receiving commands
char PRYSMA_CONFIG_TOPIC[50];       // for sending config info

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

  setupMqttClient();
}

void setupMqttClient() {
  pubSubClient.setCallback(onMqttMessage);
  snprintf(PRYSMA_CONNECTED_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, PRYSMA_ID, MQTT_CONNECTED);
  snprintf(PRYSMA_EFFECT_LIST_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, PRYSMA_ID, MQTT_EFFECT_LIST);
  snprintf(PRYSMA_STATE_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, PRYSMA_ID, MQTT_STATE);
  snprintf(PRYSMA_COMMAND_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, PRYSMA_ID, MQTT_COMMAND);
  snprintf(PRYSMA_CONFIG_TOPIC, sizeof(PRYSMA_CONNECTED_TOPIC), "%s/%s/%s",
           MQTT_TOP, PRYSMA_ID, MQTT_CONFIG);
  Serial.printf("[INFO]: Connected Topic - %s\n", PRYSMA_CONNECTED_TOPIC);
  Serial.printf("[INFO]: Effect List Topic - %s\n", PRYSMA_EFFECT_LIST_TOPIC);
  Serial.printf("[INFO]: State Topic - %s\n", PRYSMA_STATE_TOPIC);
  Serial.printf("[INFO]: Command Topic - %s\n", PRYSMA_COMMAND_TOPIC);
  Serial.printf("[INFO]: Config Topic - %s\n", PRYSMA_CONFIG_TOPIC);
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

boolean connectToMqtt() {
  PrysmaMDNS::MqttBroker mqttBroker = PrysmaMDNS::findMqttBroker();

  if (!mqttBroker.wasFound) {
    Serial.println("[WARNING]: MQTT Broker Not Found");
    return false;
  }

  pubSubClient.setServer(mqttBroker.ip, mqttBroker.port);

  if (pubSubClient.connect(PRYSMA_ID, "pi", "MQTTIsBetterThanUDP")) {
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
void loop() {
  PrysmaOTA::handleOTA();

  // If not connected, attempt to make a connection every 5 seconds
  if (!pubSubClient.connected()) {
    long now = millis();
    if (now - lastMqttConnectionAttempt > 5000) {
      Serial.println("[INFO]: Attempting MQTT connection...");
      lastMqttConnectionAttempt = now;
      // Attempt to reconnect
      if (connectToMqtt()) {
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
