/*
    ██████╗ ██████╗ ██╗   ██╗███████╗███╗   ███╗ █████╗ 
    ██╔══██╗██╔══██╗╚██╗ ██╔╝██╔════╝████╗ ████║██╔══██╗
    ██████╔╝██████╔╝ ╚████╔╝ ███████╗██╔████╔██║███████║
    ██╔═══╝ ██╔══██╗  ╚██╔╝  ╚════██║██║╚██╔╝██║██╔══██║
    ██║     ██║  ██║   ██║   ███████║██║ ╚═╝ ██║██║  ██║
    ╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝
 */
#include <ESP8266WiFi.h>

#include "PrysmaWifi.h";
#include "PrysmaOTA.h";

char PRYSMA_ID[19];
byte mac[6];

void setup()
{
  Serial.begin(115200);

  // Generate this light's unique ID from the mac address
  WiFi.macAddress(mac);
  snprintf(PRYSMA_ID, sizeof(PRYSMA_ID), "Prysma-%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("[INFO]: %s Booting\n", PRYSMA_ID);

  // Connect to WiFi
  setupWifi(PRYSMA_ID);

  // Configure Over the air uploads
  setupOTA();

  Serial.printf("[INFO]: %s Ready\n", PRYSMA_ID);
  
}

void loop()
{
  handleOTA();
}
