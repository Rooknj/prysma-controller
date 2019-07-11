#include "PrysmaWifi.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "*****"
#define STAPSK "*****"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

void setupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Connected! Yee!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}