/*
  Light.h - Library for controlling LED lights
*/
#ifndef Light_h
#define Light_h

#include <Arduino.h>
#define FASTLED_INTERNAL  // Disable pragma messages
#include <FastLED.h>

#define NO_EFFECT "None"

typedef struct {
  bool on;
  byte brightness;
  CRGB color;
  String effect;
  byte speed;
} LightState;

class Light {
 private:
  unsigned int numEffects = 3;
  String effectList[3] = {"Effect 1", "Effect 2", "Effect 3"};
  CRGB leds[512];
  int numLeds;
  byte maxBrightness;
  LightState state;
  void setTargetColor(CRGB color);
  void setTargetBrightness(byte brightness);

 public:
  Light();
  void init(int numLeds, char* stripType, char* colorOrder, int dataPin,
            int clockPin, byte maxBrightness);
  void loop();
  void identify();
  void turnOn();
  void turnOff();
  void setBrightness(byte brightness);
  void setColor(CRGB color);
  void setEffect(String effect);
  void setSpeed(byte speed);
  LightState getState();
  unsigned int getNumEffects();
  String* getEffectList();
};

#endif
