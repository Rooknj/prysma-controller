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

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");
  setupWifi();
  setupOTA();

  Serial.println("Ready");
  
}

void loop()
{
  handleOTA();
}
