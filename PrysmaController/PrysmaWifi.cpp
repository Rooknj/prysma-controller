#include "PrysmaWifi.h"
#include <Arduino.h>     // Enables use of Arduino specific functions and types
#include <ESP8266WiFi.h> // ESP8266 Core WiFi Library
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager WiFi Configuration Magic

using namespace PrysmaWifi;

void PrysmaWifi::setupWifi(char *accessPointName)
{
  // Autoconnect to Wifi
  WiFiManager wifiManager;

  // Set static IP address if one is provided
  // #ifdef STATIC_IP
  //   Serial.print("INFO: adding static IP ");
  //   Serial.println(STATIC_IP);
  //   IPAddress _ip, _gw, _sn;
  //   _ip.fromString(STATIC_IP);
  //   _gw.fromString(STATIC_GW);
  //   _sn.fromString(STATIC_SN);
  //   wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  // #endif

  // Turn the built in LED on when not connected to WIFI
  digitalWrite(LED_BUILTIN, LOW);

  if (!wifiManager.autoConnect(accessPointName))
  {
    Serial.println("[ERROR]: failed to connect to Wifi");
    Serial.println("[DEBUG]: try resetting the module");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  // Turn the built in LED off when connected to WIFI
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("[INFO]: Connected to Wifi :)");
  Serial.print("[INFO]: IP address: ");
  Serial.println(WiFi.localIP());
}
