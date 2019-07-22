#include "PrysmaMQTT.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>   // Enables finding addresses in the .local domain
#include <PubSubClient.h>  // MQTT client library

// Local Variables
WiFiClient wifiClient;
void (*connectCallback)();
typedef struct {
  bool wasFound;
  String hostname;
  IPAddress ip;
  uint16_t port;
} MqttBroker;

// Header Definitions
// TODO: if MQTT_MAX_PACKET_SIZE is less than 512, display a warning that this
// will cause errors
PubSubClient mqttClient(wifiClient);
char CONNECTED_TOPIC[50];           // for sending connection messages
char EFFECT_LIST_TOPIC[50];         // for sending the effect list
char STATE_TOPIC[50];               // for sending the state
char COMMAND_TOPIC[50];             // for receiving commands
char CONFIG_TOPIC[50];              // for sending config info
char DISCOVERY_TOPIC[50];           // for sending config info
char DISCOVERY_RESPONSE_TOPIC[50];  // for sending config info
char IDENTIFY_TOPIC[50];            // for sending config info

void setupMqttTopics(char* id) {
  snprintf(CONNECTED_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, id,
           MQTT_CONNECTED);
  Serial.printf("[INFO]: Connected Topic - %s\n", CONNECTED_TOPIC);
  snprintf(EFFECT_LIST_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, id,
           MQTT_EFFECT_LIST);
  Serial.printf("[INFO]: Effect List Topic - %s\n", EFFECT_LIST_TOPIC);
  snprintf(STATE_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, id,
           MQTT_STATE);
  Serial.printf("[INFO]: State Topic - %s\n", STATE_TOPIC);
  snprintf(COMMAND_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, id,
           MQTT_COMMAND);
  Serial.printf("[INFO]: Command Topic - %s\n", COMMAND_TOPIC);
  snprintf(CONFIG_TOPIC, sizeof(CONNECTED_TOPIC), "%s/%s/%s", MQTT_TOP, id,
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
}

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

boolean connectToMQTT(const char* id, const char* user, const char* pass,
                      const char* willTopic, uint8_t willQos,
                      boolean willRetain, const char* willMessage) {
  // Find and set the mqtt broker
  MqttBroker mqttBroker = findMqttBroker();
  if (!mqttBroker.wasFound) {
    Serial.println("[WARNING]: MQTT Broker Not Found");
    return false;
  }
  Serial.println("[INFO]: Attempting connection to MQTT broker at " +
                 mqttBroker.hostname + "...");
  mqttClient.setServer(mqttBroker.ip, mqttBroker.port);

  if (mqttClient.connect(id, user, pass, willTopic, willQos, willRetain,
                         willMessage)) {
    Serial.println("[INFO]: Connected to MQTT broker at " +
                   mqttBroker.hostname + " - " + mqttBroker.ip.toString() +
                   ":" + mqttBroker.port);

    connectCallback();
  }
  return mqttClient.connected();
}

long lastMqttConnectionAttempt = 0;
void handleMqtt(const char* id, const char* user, const char* pass,
                const char* willTopic, uint8_t willQos, boolean willRetain,
                const char* willMessage) {
  // If not connected, attempt to make a connection every 5 seconds
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastMqttConnectionAttempt > 5000) {
      lastMqttConnectionAttempt = now;
      // Attempt to reconnect
      if (connectToMQTT(id, user, pass, willTopic, willQos, willRetain,
                        willMessage)) {
        lastMqttConnectionAttempt = 0;
      } else {
        Serial.print("[WARNING]: Failed MQTT Connection, rc=");
        Serial.println(mqttClient.state());
        Serial.println("[INFO]: Attempting again in 5 seconds");
      }
    }
  } else {
    mqttClient.loop();
  }
}

void onMqttConnect(void (*callback)()) { connectCallback = callback; }

void onMqttMessage(MQTT_CALLBACK_SIGNATURE) {
  mqttClient.setCallback(callback);
}