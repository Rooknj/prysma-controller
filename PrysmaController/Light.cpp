#include "Light.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <FastLED.h>

Light::Light() {}

void Light::init(int numLeds, char* stripType, char* colorOrder, int dataPin,
                 int clockPin, byte maxBrightness) {
  this->numLeds = numLeds;
  this->maxBrightness = maxBrightness;

  FastLED.addLeds<WS2812B, 5, GRB>(this->leds, this->numLeds);

  FastLED.setBrightness(25);

  // if (strcmp(colorOrder, "RGB") == 0) {
  //   Serial.println("[INFO]: Using default Color Order GRB");
  // } else if (strcmp(colorOrder, "BGR") == 0) {
  //   Serial.println("[INFO]: Using default Color Order GRB");
  // } else {
  //   Serial.println("[INFO]: Using default Color Order GRB");
  // }

  // if (strcmp(stripType, "NEOPIXEL") == 0) {
  //   Serial.println("[INFO]: Using default strip WS2812B");
  // } else if (strcmp(stripType, "WS2811") == 0) {
  //   Serial.println("[INFO]: Using default strip WS2812B");
  // } else {
  //   Serial.println("[INFO]: Using default strip WS2812B");
  // }
}

void Light::loop() {
  // Turn the LED on, then pause
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}