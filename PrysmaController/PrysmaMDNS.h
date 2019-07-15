/*
  PrysmaMDNS.h - Library for connecting Prysma-Controller to the MQTT broker and
  handling messages
*/
#ifndef PrysmaMQTT_h
#define PrysmaMQTT_h

#include <Arduino.h>
#include <ESP8266mDNS.h>  // Enables finding addresses in the .local domain

namespace PrysmaMDNS {

typedef struct {
  bool wasFound;
  String hostname;
  IPAddress ip;
  uint16_t port;
} MqttBroker;

MqttBroker findMqttBroker();

}  // namespace PrysmaMDNS

#endif