/*
  PrysmaMQTT.h - Library for connecting Prysma-Controller to the MQTT broker and handling messages
*/
#ifndef PrysmaMQTT_h
#define PrysmaMQTT_h

#include <Arduino.h>
#include <ESP8266mDNS.h> // Enables finding addresses in the .local domain

namespace PrysmaMQTT
{
char *findMqttBrokerIp();

void createMqttTopic(char *bufferVariable, char *topLevel, char *lightName, char *topic);

void setupMQTT();
}; // namespace PrysmaMQTT

#endif