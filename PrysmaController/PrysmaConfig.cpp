#include "PrysmaConfig.h"
#include <ArduinoJson.h>
#include "FS.h"

using namespace PrysmaConfig;

Config PrysmaConfig::config;

void PrysmaConfig::init() {
  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("[ERROR]: An Error has occurred while mounting SPIFFS");
    return;
  }

  // Open config.json for reading
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("[ERROR]: Failed to open config.json for reading");
    return;
  }

  // Deserialize the JSON
  StaticJsonDocument<512> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(
        F("[ERROR]: Failed to read file, using default configuration"));
  }

  // Never use a JsonDocument to store the configuration!
  // A JsonDocument is *not* a permanent storage; it's only a temporary storage
  // used during the serialization phase. See:
  // https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/
  config.numLeds = doc["numLeds"] | 60;
  config.dataPin = doc["dataPin"] | 5;
  config.clockPin = doc["clockPin"] | -1;
  config.maxBrightness = doc["maxBrightness"] | 255;
  // We need to use strlcpy to copy the config info from doc instead of just having a pointer to it
  // If we dont, the config info will be lost partway through running the program causing strange behavior
  strlcpy(config.stripType,                    // <- destination
          doc["stripType"] | "WS2812B",        // <- source
          sizeof(config.stripType));           // <- destination's capacity
  strlcpy(config.colorOrder,                   // <- destination
          doc["colorOrder"] | "GRB",           // <- source
          sizeof(config.colorOrder));          // <- destination's capacity
  strlcpy(config.mqttUsername,                 // <- destination
          doc["mqttUsername"] | "",            // <- source
          sizeof(config.mqttUsername));        // <- destination's capacity
  strlcpy(config.mqttPassword,                 // <- destination
          doc["mqttPassword"] | "",            // <- source
          sizeof(config.mqttPassword));        // <- destination's capacity
  strlcpy(config.controllerHardware,           // <- destination
          "ESP8266",                           // <- source
          sizeof(config.controllerHardware));  // <- destination's capacity

  configFile.close();

  Serial.printf("[INFO]: numLeds - %i\n", config.numLeds);
  Serial.printf("[INFO]: dataPin - %i\n", config.dataPin);
  Serial.printf("[INFO]: clockPin - %i\n", config.clockPin);
  Serial.printf("[INFO]: maxBrightness - %i\n", config.maxBrightness);
  Serial.printf("[INFO]: stripType - %s\n", config.stripType);
  Serial.printf("[INFO]: colorOrder - %s\n", config.colorOrder);
  Serial.printf("[INFO]: controllerHardware - %s\n", config.controllerHardware);
  Serial.printf("[INFO]: mqttUsername - %s\n", config.mqttUsername);
  Serial.printf("[INFO]: mqttPassword - %s\n", config.mqttPassword);
}
