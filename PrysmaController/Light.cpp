#include "Light.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <FastLED.h>

//************************************************************************
// Light Public Class Methods
//************************************************************************
Light::Light() {}

void Light::init(int numLeds, char* stripType, char* colorOrder, int dataPin,
                 int clockPin, byte maxBrightness) {
  this->numLeds = numLeds;
  this->maxBrightness = maxBrightness;

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

  // Set the initial brightness
  FastLED.setBrightness(maxBrightness);
  // Clear the LEDs
  fill_solid(this->leds, this->numLeds, CRGB::Black);
  FastLED.show();

  // Initialize the color to the current state
  fill_solid(this->leds, this->numLeds, this->state.color);
}

void Light::loop() {
  // Handle Brightness transitions
  handleBrightnessTransition();

  // Handle when to show the changes to the LEDs
  handleShowLeds();
}

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
  transitionBrightnessTo(this->state.brightness);
}

void Light::turnOff() {
  this->state.on = false;
  transitionBrightnessTo(0);
}

void Light::setBrightness(byte brightness) {
  this->state.brightness = brightness;
  if (this->state.on) {
    transitionBrightnessTo(brightness);
  } else {
    // If the lights are off, just set the brightness immediately. Don't need
    // to be fancy
    FastLED.setBrightness(
        map(this->targetBrightness, 0, 100, 0, this->maxBrightness));
    this->currentBrightness = this->targetBrightness;
  }
}

void Light::setColor(CRGB color) {
  this->state.color = color;
  this->state.effect = NO_EFFECT;
  if (!this->state.on) {
    turnOn();
  }
  fill_solid(this->leds, this->numLeds, color);
  FastLED.show();
}
void Light::setEffect(String effect) {
  this->state.effect = effect;
  this->state.color = CRGB(255, 255, 255);
  if (!this->state.on) {
    turnOn();
  }
}
void Light::setSpeed(byte speed) { this->state.speed = speed; }

LightState Light::getState() { return this->state; }

unsigned int Light::getNumEffects() { return this->numEffects; }

String* Light::getEffectList() { return this->effectList; }

//************************************************************************
// Transition Methods
//************************************************************************
int Light::getStep(int start, int target, int numSteps) {
  return (target - start) / numSteps;
}

int Light::getRemainder(int start, int target, int numSteps) {
  return (target - start) % numSteps;
}

int Light::getChange(int stepAmount, int remainderAmount, int currentStep,
                     int numSteps) {
  int extra = 0;
  if (((currentStep * abs(remainderAmount)) / numSteps) >
      (((currentStep - 1) * abs(remainderAmount)) / numSteps)) {
    if (stepAmount < 0 || remainderAmount < 0) {
      extra = -1;
    } else {
      extra = 1;
    }
  }

  return stepAmount + extra;
}

void Light::transitionBrightnessTo(byte brightness) {
  this->startBrightnessTransition = true;
  this->targetBrightness = brightness;
}

void Light::handleBrightnessTransition() {
  if (this->startBrightnessTransition) {
    this->startBrightnessTransition = false;
    this->inBrightnessTransition = true;
    this->currentBrightnessStep = 1;
    // Calculate the step values
    this->brightnessStepAmount =
        getStep(this->currentBrightness, this->targetBrightness,
                BRIGHTNESS_TRANSITION_STEPS);
    brightnessRemainderAmount =
        getRemainder(this->currentBrightness, this->targetBrightness,
                     BRIGHTNESS_TRANSITION_STEPS);
  }

  if (this->inBrightnessTransition) {
    unsigned long now = millis();
    unsigned long brightnessStepDuration =
        BRIGHTNESS_TRANSITION_TIME / BRIGHTNESS_TRANSITION_STEPS;
    // If its time to take a step
    if (now - this->lastBrightnessStepTime > brightnessStepDuration) {
      this->lastBrightnessStepTime = now;

      this->currentBrightness +=
          getChange(this->brightnessStepAmount, this->brightnessRemainderAmount,
                    this->currentBrightnessStep, BRIGHTNESS_TRANSITION_STEPS);
      this->currentBrightnessStep++;

      // Set the value and increment the step;
      FastLED.setBrightness(
          map(this->currentBrightness, 0, 100, 0, this->maxBrightness));
    }

    // If we have gone through all the steps, end the transition
    if (this->currentBrightnessStep > BRIGHTNESS_TRANSITION_STEPS) {
      this->inBrightnessTransition = false;
      // We call show here because Light::shouldShowLeds won't trigger on the
      // last iteration since we set inBrightnessTransition to false
      FastLED.show();
      Serial.println("Ending Brightness Transition: ");
      Serial.printf("Current Brightness: %i\n", this->currentBrightness);
      Serial.printf("target Brightness: %i\n", this->targetBrightness);
    }
  }
}

//************************************************************************
// Effect Methods
//************************************************************************
bool Light::shouldShowLeds() {
  // If you are in a brightness transition, show leds
  if (this->inBrightnessTransition) {
    return true;
  }

  // If the light is on and performing an effect, show leds
  if (this->state.on && this->state.effect != NO_EFFECT) {
    return true;
  }

  return false;
}

void Light::handleShowLeds() {
  if (!shouldShowLeds()) {
    return;
  }

  unsigned long now = millis();
  // If its time to take a step
  if (now - this->lastShowLedsTime > 1000 / FRAMES_PER_SECOND) {
    this->lastShowLedsTime = now;
    FastLED.show();
  }
}