#include "Light.h"


Light::Light(int switchPin, int LDRPin, int potPin) : OVERRIDE_DURATION(10000) {
  lightSwitch = switchPin; // Dedicated Switch #1 on Pin D3
  LDR = LDRPin;            // Dedicated LDR on Pin A0
  lightPOT = potPin;       // Dedicated Potentiometer #1 on Pin A6

  isIndoorLightOn = false;
  lastSwitchState = HIGH;
  isAutoMode = true;
  manualOverrideTimer = 0;

  lastPotValue = -1;
  isThresholdOverridden = false;
  ldrThreshold = 512;
  lastLdrValue = 500;

  matrixBrightness = MATRIX_BRIGHTNESS;
}

void Light::begin() {
  lastSwitchState = digitalRead(lightSwitch);
}

void Light::setBrightness(int newBrightness) {
  matrixBrightness = constrain(newBrightness, 0, 255);
}

int Light::getBrightness() {
  return matrixBrightness;
}

void Light::setBluetoothThreshold(int newThr) {
  ldrThreshold = constrain(newThr, 0, 1023);
  isThresholdOverridden = true;
}

int Light::getThreshold() {
  return ldrThreshold;
}

int Light::getLastLDRValue() {
  return lastLdrValue;
}

void Light::returnToPotentiometer() {
  isThresholdOverridden = false;
}

void Light::lightOn() {
  if (isIndoorLightOn) return;
  isIndoorLightOn = true;
  extern void setIndoorLight(bool on);
  setIndoorLight(true);
}

void Light::lightOff() {
  if (!isIndoorLightOn) return;
  isIndoorLightOn = false;
  extern void setIndoorLight(bool on);
  setIndoorLight(false);
}

bool Light::getIndoorLight() {
  return isIndoorLightOn;
}

bool Light::getIsAutoMode() {
  return isAutoMode;
}

void Light::enableAutoMode() {
  isAutoMode = true;
}

void Light::disableAutoMode() {
  isAutoMode = false;
  manualOverrideTimer = millis();
}

void Light::toggleLightBtn() {
  int currentSwitchState = digitalRead(lightSwitch);

  if (currentSwitchState != lastSwitchState) {
    lastSwitchState = currentSwitchState;
    disableAutoMode();

    if (currentSwitchState == LOW) {
      lightOn();
    } else {
      lightOff();
    }
  }
}

void Light::LDRCheck() {
  // Read Dedicated Potentiometer #1 on Pin A6
  int currentPotValue = analogRead(lightPOT);

  if (lastPotValue == -1 || abs(currentPotValue - lastPotValue) > 10) {
    isThresholdOverridden = false;
    lastPotValue = currentPotValue;
  }

  if (isThresholdOverridden == false) {
    ldrThreshold = currentPotValue;
  }

  if (isAutoMode == false) {
    if (millis() - manualOverrideTimer >= OVERRIDE_DURATION) {
      enableAutoMode();
    }
  }

  lastLdrValue = analogRead(LDR);

  if (isAutoMode == true) {
    if (lastLdrValue < ldrThreshold) {
      lightOn();
    } else {
      lightOff();
    }
  }
}
