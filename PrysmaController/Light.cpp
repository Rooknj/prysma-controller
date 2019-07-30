#include "Light.h"
#include <Arduino.h>  // Enables use of Arduino specific functions and types
#include <FastLED.h>
#include <WiFiUdp.h>
WiFiUDP port;

//************************************************************************
// Public Methods
//************************************************************************
Light::Light() {}

void Light::init(int numLeds, char* stripType, char* colorOrder, int dataPin,
                 int clockPin, byte maxBrightness) {
  this->numLeds = numLeds;
  this->maxBrightness = maxBrightness;

  // Start listening for UDP Packets
  port.begin(localPort);

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

  // Handle Color transitions
  handleColorTransition();

  // Handle Visualizations over UDP
  /*
    Parse the UDP Packet. This is required to be called in the loop every time
    so that the UDP buffer doesn't overflow.
    This is also being called inside the loop instead of handleEffect so that it
    can get 60FPS. Putting handleVisualize inside the handleEffect method causes
    FPS closer to 30
   */
  int packetSize = port.parsePacket();
  if (this->state.effect == "Visualize") {
    handleVisualize(packetSize);
  }

  // Handle the currently playing effect
  handleEffect();

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
  // Setting a color automatically turns the light on
  if (!this->state.on) {
    turnOn();
  }
  if (this->state.effect != NO_EFFECT) {
    // If an effect is playing, just go straight to the color, no transition
    this->state.effect = NO_EFFECT;
    this->currentColor = color;
    fill_solid(this->leds, this->numLeds, color);
  } else {
    transitionColorTo(color);
  }
}

void Light::setEffect(String effect) {
  this->state.effect = effect;
  this->state.color = CRGB(255, 255, 255);
  // Clear the lights when setting an effect
  fill_solid(this->leds, this->numLeds, CRGB::Black);
  FastLED.show();
  // Setting an effect automatically turns the light on
  if (!this->state.on) {
    turnOn();
  }
}

void Light::setSpeed(byte speed) { this->state.speed = speed; }

LightState Light::getState() { return this->state; }

unsigned int Light::getNumEffects() { return this->numEffects; }

String* Light::getEffectList() { return this->effectList; }

//************************************************************************
// Transitions
//************************************************************************
// General
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

// Brightness
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

      // Calculate the amount to change brightness by
      this->currentBrightness +=
          getChange(this->brightnessStepAmount, this->brightnessRemainderAmount,
                    this->currentBrightnessStep, BRIGHTNESS_TRANSITION_STEPS);

      // Set the value and increment the step;
      FastLED.setBrightness(
          map(this->currentBrightness, 0, 100, 0, this->maxBrightness));
      this->currentBrightnessStep++;
    }

    // If we have gone through all the steps, end the transition
    if (this->currentBrightnessStep > BRIGHTNESS_TRANSITION_STEPS) {
      this->inBrightnessTransition = false;
      // We call show here because Light::shouldShowLeds won't trigger on the
      // last iteration since we set inBrightnessTransition to false
      FastLED.show();
      // Serial.println("Ending Brightness Transition: ");
      // Serial.printf("Current Brightness: %i\n", this->currentBrightness);
      // Serial.printf("target Brightness: %i\n", this->targetBrightness);
    }
  }
}

// Color
void Light::transitionColorTo(CRGB color) {
  this->startColorTransition = true;
  this->targetColor = color;
}

void Light::handleColorTransition() {
  if (this->startColorTransition) {
    this->startColorTransition = false;
    this->inColorTransition = true;
    this->currentColorStep = 1;
    // Calculate the step values
    this->redStepAmount = getStep(this->currentColor.r, this->targetColor.r,
                                  COLOR_TRANSITION_STEPS);
    this->greenStepAmount = getStep(this->currentColor.g, this->targetColor.g,
                                    COLOR_TRANSITION_STEPS);
    this->blueStepAmount = getStep(this->currentColor.b, this->targetColor.b,
                                   COLOR_TRANSITION_STEPS);
    this->redRemainderAmount = getRemainder(
        this->currentColor.r, this->targetColor.r, COLOR_TRANSITION_STEPS);
    this->greenRemainderAmount = getRemainder(
        this->currentColor.g, this->targetColor.g, COLOR_TRANSITION_STEPS);
    this->blueRemainderAmount = getRemainder(
        this->currentColor.b, this->targetColor.b, COLOR_TRANSITION_STEPS);
  }

  if (this->inColorTransition) {
    unsigned long now = millis();
    unsigned long stepDuration = COLOR_TRANSITION_TIME / COLOR_TRANSITION_STEPS;
    // If its time to take a step
    if (now - this->lastColorStepTime > stepDuration) {
      this->lastColorStepTime = now;

      // Calculate the next value to change to
      this->currentColor.r +=
          getChange(this->redStepAmount, this->redRemainderAmount,
                    this->currentColorStep, COLOR_TRANSITION_STEPS);
      this->currentColor.g +=
          getChange(this->greenStepAmount, this->greenRemainderAmount,
                    this->currentColorStep, COLOR_TRANSITION_STEPS);
      this->currentColor.b +=
          getChange(this->blueStepAmount, this->blueRemainderAmount,
                    this->currentColorStep, COLOR_TRANSITION_STEPS);

      // Set the value and increment the step;
      fill_solid(this->leds, this->numLeds, this->currentColor);
      this->currentColorStep++;

      // If we have gone through all the steps, end the transition
      if (this->currentColorStep > COLOR_TRANSITION_STEPS) {
        this->inColorTransition = false;
        // We call show here because Light::shouldShowLeds won't trigger on the
        // last iteration since we set inColorTransition to false
        FastLED.show();
        // Serial.println("Ending Color Transition: ");
        // Serial.printf("Current Red: %i, Current Green: %i, Current Blue:
        // %i\n",
        //               this->currentColor.r, this->currentColor.g,
        //               this->currentColor.b);
        // Serial.printf("target Red: %i, target Green: %i, target Blue: %i\n",
        //               this->targetColor.r, this->targetColor.g,
        //               this->targetColor.b);
      }
    }
  }
}

//************************************************************************
// Effects
//************************************************************************
// General
bool Light::shouldShowLeds() {
  // If you are in a brightness transition, show leds
  if (this->inBrightnessTransition) {
    return true;
  }

  // If you are in a color transition, show leds
  if (this->inColorTransition) {
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

bool Light::shouldUpdateEffect() {
  // Don't update the effect if the light is off or currently transitioning to
  // be off
  if (!this->state.on && !this->inBrightnessTransition) {
    return false;
  }

  String currentEffect = this->state.effect;

  // ADD_EFFECT: Add the appropriate update threshold if the default speeds are
  // inadequate
  unsigned long updateThreshold;
  if (currentEffect == "Flash") {
    updateThreshold = this->FLASH_SPEEDS[this->state.speed - 1];
  } else if (currentEffect == "Juggle" || currentEffect == "Fire" ||
             currentEffect == "Blue Noise") {
    updateThreshold = 17;
  } else {
    updateThreshold = this->DEFAULT_SPEEDS[this->state.speed - 1];
  }

  unsigned long now = millis();
  if (now - this->lastUpdateEffectTime > updateThreshold) {
    this->lastUpdateEffectTime = now;
    return true;
  }
  return false;
}

void Light::handleEffect() {
  String currentEffect = this->state.effect;
  if (currentEffect == NO_EFFECT) {
    return;
  }

  if (!shouldUpdateEffect()) {
    return;
  }

  // ADD_EFFECT: Add the effect to this handler
  if (currentEffect == "Flash") {
    handleFlash();
  } else if (currentEffect == "Fade") {
    handleFade();
  } else if (currentEffect == "Confetti") {
    handleConfetti();
  } else if (currentEffect == "Juggle") {
    handleJuggle();
  } else if (currentEffect == "Rainbow") {
    handleRainbow();
  } else if (currentEffect == "Cylon") {
    handleCylon();
  } else if (currentEffect == "Fire") {
    handleFire();
  } else if (currentEffect == "Blue Noise") {
    handleBlueNoise();
  }
}

void Light::cycleHue() { this->gHue++; }

// Flash
void Light::handleFlash() {
  switch (this->flashColor) {
    case 0: {
      fill_solid(this->leds, this->numLeds, CRGB::Red);
      this->flashColor = 1;
      break;
    }
    case 1: {
      fill_solid(this->leds, this->numLeds, CRGB::Green);
      this->flashColor = 2;
      break;
    }
    case 2: {
      fill_solid(this->leds, this->numLeds, CRGB::Blue);
      this->flashColor = 0;
      break;
    }
  }
}

// Fade
void Light::handleFade() {
  cycleHue();
  fill_solid(this->leds, this->numLeds, CHSV(gHue, 255, 255));
}

// Confetti
void Light::handleConfetti() {
  cycleHue();
  fadeToBlackBy(this->leds, this->numLeds, 10);
  int pos = random16(this->numLeds);
  this->leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

// Juggle
void Light::handleJuggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(this->leds, this->numLeds,
                this->JUGGLE_FADE[this->state.speed - 1]);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    this->leds[beatsin16(i + this->JUGGLE_BPMS_ADDER[this->state.speed - 1], 0,
                         this->numLeds - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// Rainbow
void Light::handleRainbow() {
  cycleHue();
  // The shorter the last number, the longer each color is on the rainbow
  fill_rainbow(this->leds, this->numLeds, this->gHue, 3);
}

// Cylon
void Light::handleCylon() {
  cycleHue();
  for (int i = 0; i < this->numLeds; i++) {
    this->leds[i].nscale8(247);
  }
  // First slide the led in one direction
  if (this->cylonLed >= this->numLeds - 1) {
    this->cylonForward = false;
  } else if (this->cylonLed <= 0) {
    this->cylonForward = true;
  }
  if (this->cylonForward) {
    this->cylonLed++;
  } else {
    this->cylonLed--;
  }
  this->leds[this->cylonLed] = CHSV(this->gHue, 255, 255);
}

// Fire
void Light::handleFire() {
  // Step 1.  Cool down every cell a little
  for (int i = 0; i < this->numLeds; i++) {
    this->heat[i] = qsub8(
        this->heat[i], random8(0, ((this->COOLING * 10) / this->numLeds) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = this->numLeds - 1; k >= 2; k--) {
    this->heat[k] =
        (this->heat[k - 1] + this->heat[k - 2] + this->heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of this->heat near the bottom
  if (random8() < this->SPARKING) {
    int y = random8(7);
    this->heat[y] = qadd8(this->heat[y], random8(160, 255));
  }

  // Step 4.  Map from this->heat cells to LED colors
  for (int j = 0; j < this->numLeds; j++) {
    // Scale the this->heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8(this->heat[j], 240);
    CRGB color = ColorFromPalette(HeatColors_p, colorindex);
    int pixelnumber;
    if (this->fireReverseDirection) {
      pixelnumber = (this->numLeds - 1) - j;
    } else {
      pixelnumber = j;
    }
    this->leds[pixelnumber] = color;
  }
}

// Blue Noise
void Light::handleBlueNoise() {
  // Just one loop to fill up the LED array as all of the pixels change.
  for (int i = 0; i < this->numLeds; i++) {
    // Get a value from the noise function. I'm using both x and y axis.
    uint8_t index =
        inoise8(i * this->scale, this->dist + i * this->scale) % 255;
    // With that value, look up the 8 bit colour palette
    // value and assign it to the current LED.
    this->leds[i] = ColorFromPalette(OceanColors_p, index, 255, LINEARBLEND);
  }
  // Moving along the distance (that random number we started out with). Vary it
  // a bit with a sine wave.
  this->dist += beatsin8(10, 2, 5);
}

void Light::handleVisualize(int packetSize) {
  unsigned int expectedPacketSize = this->numLeds * 3;
  // If packets have been received, interpret the command
  if (packetSize == expectedPacketSize) {
    port.read((char*)this->leds, sizeof(this->leds));
#if PRINT_FPS
    this->fpsCounter++;
    Serial.print("/");  // Monitors connection(shows jumps/jitters in packets)
#endif
  } else if (packetSize) {
    Serial.printf("Invalid packet size: %u (expected %u)\n", packetSize,
                  expectedPacketSize);
    port.flush();
    return;
  }

#if PRINT_FPS
  if (millis() - this->secondTimer >= 1000U) {
    this->secondTimer = millis();
    Serial.printf("FPS: %d\n", this->fpsCounter);
    this->fpsCounter = 0;
  }
#endif
}
// ADD_EFFECT: Add the effect handler code below