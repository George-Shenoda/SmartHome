#ifndef DOORCONTROL_H
#define DOORCONTROL_H

#include <Arduino.h>
#include <Servo.h>
#include <Keypad.h>

class DoorControl {
  private:
    Servo myservo;
    int servoPin;
    String password;
    String typed;
    bool isDoorOpen;
    unsigned long doorOpenTimer;
    const unsigned long DOOR_OPEN_DURATION;
    // Anti-Buzz Servo Detach Timer (detaches SG90 600ms after reaching target angle)
    bool isServoAttached;
    unsigned long servoAttachTimer;
    const unsigned long SERVO_MOVE_TIME = 600;
    const int ANGLE_CLOSED = 15; // Safe angle away from 0-deg mechanical stall stop
    const int ANGLE_OPEN   = 90;

    Keypad* customkeypad;
    char lastAnalogKey;
    char stableKey;
    unsigned long lastKeyDebounce;

    void moveServoSmooth(int targetAngle);
    char scanAnalogColumnKeypad();

  public:
    DoorControl(int sPin, String pwd);
    void begin();
    void openDoor();
    void closeDoor();
    void update();
    bool getIsOpen();
    bool verifyPassword(String inputPwd);
    int getRemainingSeconds();
};

#endif
