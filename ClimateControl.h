#ifndef CLIMATECONTROL_H
#define CLIMATECONTROL_H

#include <Arduino.h>

class ClimateControl {
  private:
    int dhtPin;
    int motorPin; // Pin D9 -> R7 (1k) -> Q1 (2N2222A Base) driving DC Motor + Fan Propeller
    int potPin;   // Pin A7 -> Potentiometer #2 (Temp Threshold 0-55 C)
    unsigned long lastDHTReadTime;

    bool isAutoMode;
    int lastPotValue;
    bool isThresholdOverridden;

    void setDCMotorState(bool turnOn);

  public:
    ClimateControl(int dPin, int mPin, int pPin);

    void begin();
    void setBluetoothThreshold(float newThr);
    void setManualFan(bool turnOn);
    void setAutoFan();
    void returnToPotentiometer();
    bool getIsAutoMode();
    void update();
};

#endif
