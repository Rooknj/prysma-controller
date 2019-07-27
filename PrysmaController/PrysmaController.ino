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

#include "Light.h";
#include "PrysmaConfig.h"
#include "PrysmaMQTT.h";
#include "PrysmaOTA.h";
#include "PrysmaWifi.h";

#define DEBUG true
#define VERSION "2.0.0"

//*******************************************************
// Global Variables
//*******************************************************
char PRYSMA_ID[19];

char connectedMessage[50];
char disconnectedMessage[50];

char mutationId[37];  // uuidv4 (36 characters + 1)
bool mutationIdWasChanged = false;

Light light;

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
void sendState() {
  StaticJsonDocument<512> doc;

  // populate payload with mutationId if one was sent
  if (mutationIdWasChanged) {
    Serial.printf("Mutation Id: %s\n", mutationId);
    doc["mutationId"] = mutationId;
    bool mutationIdWasChanged = false;
  }

  doc["id"] = PRYSMA_ID;

  LightState state = light.getState();
  doc["on"] = state.on;
  doc["brightness"] = state.brightness;
  JsonObject color = doc.createNestedObject("color");
  color["r"] = state.color.r;
  color["g"] = state.color.g;
  color["b"] = state.color.b;
  doc["effect"] = state.effect;
  doc["speed"] = state.speed;

  char stateMessage[512];
  serializeJson(doc, stateMessage);

  mqttClient.publish(STATE_TOPIC, stateMessage);
  Serial.printf("[INFO]: Published %s to <%s>\n", stateMessage, STATE_TOPIC);
}

// Send the list of supported effects via MQTT
void sendEffectList() {
  Serial.println("Send Effect List");
  StaticJsonDocument<512> doc;
  doc["id"] = PRYSMA_ID;
  JsonArray effectList = doc.createNestedArray("effectList");
  String *effects = light.getEffectList();
  for (int i = 0; i < light.getNumEffects(); i++) {
    effectList.add(effects[i]);
  }
  
  char effectListMessage[512];
  serializeJson(doc, effectListMessage);

  mqttClient.publish(EFFECT_LIST_TOPIC, effectListMessage);
  Serial.printf("[INFO]: Published %s to <%s>\n", effectListMessage,
                EFFECT_LIST_TOPIC);
}

// Send the config of the light via MQTT
void sendConfig(boolean discoveryResponse = false) {
  StaticJsonDocument<512> doc;
  doc["id"] = PRYSMA_ID;
  doc["version"] = VERSION;
  doc["hardware"] = config.controllerHardware;
  doc["colorOrder"] = config.colorOrder;
  doc["stripType"] = config.stripType;
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["macAddress"] = WiFi.macAddress();
  doc["numLeds"] = config.numLeds;
  doc["udpPort"] = 7778;

  char configMessage[512];
  serializeJson(doc, configMessage);

  if (discoveryResponse) {
    // Send a one time message to the discovery response (dont retain the
    // message)
    mqttClient.publish(DISCOVERY_RESPONSE_TOPIC, configMessage);
    Serial.printf("[INFO]: Published %s to <%s>\n", configMessage,
                  DISCOVERY_RESPONSE_TOPIC);
  } else {
    mqttClient.publish(CONFIG_TOPIC, configMessage, true);
    Serial.printf("[INFO]: Published %s to <%s>\n", configMessage,
                  CONFIG_TOPIC);
  }
}

// Respond to a discovery query with the config information of the light
void sendDiscoveryResponse() { sendConfig(true); }

// Deal with a message on the command topic
void handleCommand(byte *payload) {
  Serial.println("[INFO]: Handling Command Message");

  // Parse JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("[ERROR]: Failed to parse config message JSON");
    return;
  }
  // Pretty print JSON
  serializeJsonPretty(doc, Serial);
  Serial.println();  // Add a linebreak to the end

  // Update the next sendState response with the mutationId
  if (doc.containsKey("mutationId") &&
      strcmp(doc["mutationId"], mutationId) != 0) {
    strlcpy(mutationId,           // <- destination
            doc["mutationId"],    // <- source
            sizeof(mutationId));  // <- destination's capacity
    mutationIdWasChanged = true;
  }

  // Handle the actual commands
  if (doc.containsKey("on")) {
    bool on = doc["on"];
    if (on) {
      light.turnOn();
    } else {
      light.turnOff();
    }
  }

  if (doc.containsKey("brightness")) {
    byte brightness = doc["brightness"];
    light.setBrightness(brightness);
  }

  if (doc.containsKey("color")) {
    byte r = doc["color"]["r"];
    byte g = doc["color"]["g"];
    byte b = doc["color"]["b"];
    light.setColor(CRGB(r, g, b));
  }

  if (doc.containsKey("effect")) {
    String effect = doc["effect"];
    light.setEffect(effect);
  }

  if (doc.containsKey("speed")) {
    byte speed = doc["speed"];
    light.setSpeed(speed);
  }

  sendState();
}

// Deal with a discovery query
void handleDiscovery() {
  Serial.println("[INFO]: Handling Discovery Message");
  sendDiscoveryResponse();
}

// Deal with an identify command
void handleIdentify() {
  Serial.println("[INFO]: Handling Identify Message");
  light.identify();
}

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
  byte mac[6];
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

  // Initialize the light
  light.init(config.numLeds, config.stripType, config.colorOrder,
             config.dataPin, config.clockPin, config.maxBrightness);
}

void loop() {
  handleOTA();
  handleMqtt(PRYSMA_ID, config.mqttUsername, config.mqttPassword,
             CONNECTED_TOPIC, 0, true, disconnectedMessage);
  light.loop();
}
