/*
  Light.h - Library for controlling LED lights
*/
#ifndef Light_h
#define Light_h

#include <Arduino.h>
#define FASTLED_INTERNAL  // Disable pragma messages
#include <FastLED.h>

#define NO_EFFECT "None"
#define FRAMES_PER_SECOND 60
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 100
#define BRIGHTNESS_TRANSITION_TIME 500
#define BRIGHTNESS_TRANSITION_STEPS 30
#define COLOR_TRANSITION_TIME 500
#define COLOR_TRANSITION_STEPS 30

typedef struct {
  bool on;
  byte brightness;
  CRGB color;
  String effect;
  byte speed;
} LightState;

class Light {
 private:
  LightState state = {false, 100, CRGB(255, 0, 0), NO_EFFECT, 4};
  // FastLED variables
  CRGB leds[512];
  int numLeds;
  byte maxBrightness;
  // Transitions: General
  int getStep(int start, int target, int numSteps);
  int getRemainder(int start, int target, int numSteps);
  int getChange(int stepAmount, int remainderAmount, int currentStep,
                int numSteps);
  // Transitions: Brightness
  unsigned long lastBrightnessStepTime = 0;
  byte currentBrightness = 0;
  byte targetBrightness;
  int currentBrightnessStep;
  int brightnessStepAmount;
  int brightnessRemainderAmount;
  bool startBrightnessTransition;
  bool inBrightnessTransition;
  void transitionBrightnessTo(byte brightness);
  void handleBrightnessTransition();
  // Transitions: Color
  unsigned long lastColorStepTime = 0;
  CRGB currentColor = CRGB(255, 0, 0);
  CRGB targetColor;
  int currentColorStep;
  int redStepAmount;
  int greenStepAmount;
  int blueStepAmount;
  int redRemainderAmount;
  int greenRemainderAmount;
  int blueRemainderAmount;
  bool startColorTransition;
  bool inColorTransition;
  void transitionColorTo(CRGB color);
  void handleColorTransition();
  // Effect List Variables
  unsigned int numEffects = 3;
  String effectList[3] = {"Effect 1", "Effect 2", "Effect 3"};
  // Effects: General
  unsigned long lastShowLedsTime = 0;
  bool shouldShowLeds();
  void handleShowLeds();

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
