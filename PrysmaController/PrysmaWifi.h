/*
  PrysmaWifi.h - Library for connecting Prysma-Controller to WiFi
*/
#ifndef PrysmaWifi_h
#define PrysmaWifi_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

namespace PrysmaWifi {
void setupWifi(char *accessPointName);
};

#endif