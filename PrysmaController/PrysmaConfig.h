/*
  PrysmaConfig.h - Library for getting config information from SPIFFS
  handling messages
*/
#ifndef PrysmaConfig_h
#define PrysmaConfig_h

#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include "FS.h"

void setupConfig();

struct Config {
  int numLeds;
  int dataPin;
  int clockPin;
  int maxBrightness;
  char stripType[16];
  char colorOrder[4];
  char controllerHardware[16];
  char mqttUsername[50];
  char mqttPassword[50];
};

extern Config config;

#endif
