/*
  Light.h - Library for controlling LED lights
*/
#ifndef Light_h
#define Light_h

#include <Arduino.h>
#define FASTLED_INTERNAL // Disable pragma messages
#include <FastLED.h>

#define NO_EFFECT "None"

typedef struct {
  bool on;
  byte brightness;
  byte r;
  byte g;
  byte b;
  char* effect;
  byte speed;
} LightState;

class Light {
 private:
  // int numEffects = 3;
  // String effectList[this->numEffects] = {"Test 1", "Test 2", "Test 3"};
  String effectList[3] = {"Test 1", "Test 2", "Test 3"};
  CRGB leds[512];
  int numLeds;
  byte maxBrightness;

 public:
  Light();
  void init(int numLeds, char* stripType, char* colorOrder, int dataPin, int clockPin, byte maxBrightness);
  void loop();
  void setOn(bool on);
  void setBrightness(byte brightness);
  void setColor(byte r, byte g, byte b);
  void setEffect(char* effect);
  void setSpeed(byte speed);
  LightState getState();
  String* getEffectList();
};

#endif
