#include "PrysmaMQTT.h"
#include <Arduino.h>     // Enables use of Arduino specific functions and types
#include <ESP8266mDNS.h> // Enables finding addresses in the .local domain

using namespace PrysmaMQTT;

long lastQueryAttempt = 0;
char *PrysmaMQTT::findMqttBrokerIp()
{
  // Find all mqtt service advertisements over MDNS
  int n = MDNS.queryService("mqtt", "tcp");

  // If none were found, return null;
  if (n == 0)
  {
    Serial.println("[INFO]: No MQTT services found");
    return NULL;
  }

  // Loop through all the services found and pick the best one
  Serial.println("--- MDNS Query Results ---");
  for (int i = 0; i < n; ++i)
  {
    String SERVICE_NAME = MDNS.hostname(i);
    IPAddress SERVICE_IP = MDNS.IP(i);
    uint16_t SERVICE_PORT = MDNS.port(i);
    Serial.printf("[INFO]: MDNS Result %i:\n", i);
    Serial.println("[INFO]: SERVICE Hostname - " + SERVICE_NAME);
    Serial.println("[INFO]: SERVICE Host IP - " + SERVICE_IP.toString());
    Serial.printf("[INFO]: SERVICE Port - %i\n", SERVICE_PORT);
    Serial.println("------");

    // Services at prysma.local take priority
    if (SERVICE_NAME.indexOf("prysma") >= 0)
    {
      char mqttBrokerIp[16];
      SERVICE_IP.toString().toCharArray(mqttBrokerIp, sizeof(mqttBrokerIp));
      return mqttBrokerIp;
    }
  }

  // If there were no services with the hostname prysma.local, just return the first ip address that came up
  char mqttBrokerIp[16];
  MDNS.IP(0).toString().toCharArray(mqttBrokerIp, sizeof(mqttBrokerIp));
  return mqttBrokerIp;
}

void PrysmaMQTT::setupMQTT()
{
}
