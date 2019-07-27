#include "Light.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <FastLED.h>

//************************************************************************
// Light Public Class Methods
//************************************************************************
Light::Light() { this->state = {false, 100, CRGB(255, 0, 0), NO_EFFECT, 4}; }

void Light::init(int numLeds, char* stripType, char* colorOrder, int dataPin,
                 int clockPin, byte maxBrightness) {
  this->numLeds = numLeds;
  this->maxBrightness = maxBrightness;

  FastLED.setBrightness(maxBrightness);

  // Initialize the leds
  // Currently I need to specify all these combinations manually since FastLED
  // requires all variables in the template to be constants
  if (clockPin > 0) {
    Serial.println("[INFO]: Using 4 pin default APA102 + RGB");
    FastLED.addLeds<APA102, 5, 6, RGB>(this->leds, this->numLeds);
  } else if (strcmp(stripType, "WS2811") == 0) {
    Serial.println("[INFO]: Using WS2811 + GRB");
    FastLED.addLeds<WS2811, 5, GRB>(this->leds, this->numLeds);
  } else {
    Serial.println("[INFO]: Using default WS2812B + GRB");
    FastLED.addLeds<WS2812B, 5, GRB>(this->leds, this->numLeds);
  }
}

void Light::loop() {}

void Light::identify() {
  // TODO: Figure out how to do this without delays
  fill_solid(this->leds, this->numLeds, CRGB::Green);
  FastLED.show();
  FastLED.delay(500);
  fill_solid(this->leds, this->numLeds, CRGB::Black);
  FastLED.show();
  FastLED.delay(500);
  fill_solid(this->leds, this->numLeds, CRGB::Green);
  FastLED.show();
  FastLED.delay(500);
  // TODO: make this return the light to it's previous state
  fill_solid(this->leds, this->numLeds, CRGB::Black);
  FastLED.show();
  FastLED.delay(500);
}

void Light::turnOn() {
  this->state.on = true;
  fill_solid(this->leds, this->numLeds, this->state.color);
  FastLED.show();
}

void Light::turnOff() {
  this->state.on = false;
  fill_solid(this->leds, this->numLeds, CRGB::Black);
  FastLED.show();
}

void Light::setBrightness(byte brightness) {
  this->state.brightness = brightness;
  FastLED.setBrightness(map(brightness, 0, 100, 0, this->maxBrightness));
  FastLED.show();
}

void Light::setColor(CRGB color) {
  this->state.on = true;
  this->state.color = color;
  fill_solid(this->leds, this->numLeds, color);
  FastLED.show();
}
void Light::setEffect(String effect) { this->state.effect = effect; }
void Light::setSpeed(byte speed) { this->state.speed = speed; }

LightState Light::getState() { return this->state; }

unsigned int Light::getNumEffects() { return this->numEffects; }

String* Light::getEffectList() { return this->effectList; }

//************************************************************************
// Crossfade
//************************************************************************