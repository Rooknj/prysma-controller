/*
  PrysmaOTA.h - Library for handling Over The Air uploads to Prysma-Controller
*/
#ifndef PrysmaOTA_h
#define PrysmaOTA_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

namespace PrysmaOTA {
void setupOTA(char *hostname);

void handleOTA();
}  // namespace PrysmaOTA
#endif
