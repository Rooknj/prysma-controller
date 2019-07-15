#include "FS.h"

#define VERSION "1.0.0"

void test() {
  if (!SPIFFS.begin()) {
    Serial.println("[ERROR]: An Error has occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("[ERROR]: Failed to open file for reading");
    return;
  }

  Serial.println();
  Serial.println("[INFO]: File Content:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}
