#include "ClimateControl.h"
#include <dht.h>

extern dht DHT;
extern float currentTemp;
extern float currentHum;
extern float tempThreshold;
extern bool isFanOn;

ClimateControl::ClimateControl(int dPin, int mPin, int pPin) {
  dhtPin = dPin;
  motorPin = mPin; // Pin D9 driving DC Motor + Fan Propeller via NPN Transistor Q1
  potPin = pPin;   // Dedicated Potentiometer #2 on Arduino Nano Pin A7
  lastDHTReadTime = 0;
  isAutoMode = true;
  lastPotValue = -1;
  isThresholdOverridden = false;
}

void ClimateControl::begin() {
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); // Ensure DC Motor + Fan is OFF on startup
  isFanOn = false;
}

void ClimateControl::setDCMotorState(bool turnOn) {
  if (turnOn) {
    digitalWrite(motorPin, HIGH);
    isFanOn = true;
  } else {
    digitalWrite(motorPin, LOW);
    isFanOn = false;
  }
}

void ClimateControl::setBluetoothThreshold(float newThr) {
  tempThreshold = newThr;
  isThresholdOverridden = true;
}

void ClimateControl::setManualFan(bool turnOn) {
  isAutoMode = false;
  setDCMotorState(turnOn);
}

void ClimateControl::setAutoFan() {
  isAutoMode = true;
}

void ClimateControl::returnToPotentiometer() {
  isThresholdOverridden = false;
}

bool ClimateControl::getIsAutoMode() {
  return isAutoMode;
}

void ClimateControl::update() {
  // 1. Read DHT11 every 2000ms (non-blocking)
  if (millis() - lastDHTReadTime >= 2000) {
    lastDHTReadTime = millis();
    int chk = DHT.read11(dhtPin);
    if (chk == DHTLIB_OK) {
      currentTemp = DHT.temperature;
      currentHum = DHT.humidity;
    }
  }

  // 2. Smooth 4-Sample Read on Potentiometer #2 (Pin A7)
  long sum = 0;
  for (int i = 0; i < 4; i++) {
    sum += analogRead(potPin);
  }
  int currentPotValue = sum / 4;

  if (lastPotValue == -1 || abs(currentPotValue - lastPotValue) > 15) {
    isThresholdOverridden = false;
    lastPotValue = currentPotValue;
  }

  if (!isThresholdOverridden) {
    tempThreshold = map(currentPotValue, 0, 1023, 0, 55);
  }

  // 3. Control DC Motor + Fan Propeller (with 0.5 C hysteresis in Auto Mode to prevent chattering)
  if (isAutoMode) {
    if (!isFanOn && currentTemp > tempThreshold) {
      setDCMotorState(true);
    } else if (isFanOn && currentTemp <= (tempThreshold - 0.5)) {
      setDCMotorState(false);
    } 
  }
}
