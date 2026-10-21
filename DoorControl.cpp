#include "HardwareSerial.h"
#include "DoorControl.h"

// ============================================================================
// ARDUINO NANO 4x4 KEYPAD PIN ALLOCATION (5 PINS: D13, D12, D11, D10 + A1)
// - Rows R1..R4 connect to Digital Pins D13, D12, D11, D10.
// - Columns C1..C4 connect to Analog/Digital Pin A1 via a 4-resistor ladder
//   (C1: 0 ohm, C2: 1k, C3: 2.2k, C4: 4.7k with R9 = 1k pull-up to +5V).
// ============================================================================
extern char lastKey;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10};
const int KEYPAD_COL_PIN = A1;

DoorControl::DoorControl(int sPin, String pwd) : DOOR_OPEN_DURATION(20000) {
  servoPin = sPin;
  password = pwd;
  typed = "";
  isDoorOpen = false;
  doorOpenTimer = 0;
  isServoAttached = false;
  servoAttachTimer = 0;
  lastAnalogKey = 0;
  stableKey = 0;
  lastKeyDebounce = 0;
  customkeypad = nullptr;
  }

void DoorControl::moveServoSmooth(int targetAngle) {
  if (!isServoAttached) {
    myservo.attach(servoPin);
    isServoAttached = true;
  }
  myservo.write(targetAngle);
  servoAttachTimer = millis();
}

void DoorControl::begin() {
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  // Enable internal pull-up on A1 so an unconnected/loose wire never floats
  pinMode(KEYPAD_COL_PIN, INPUT_PULLUP);

  // Move servo to safe closed position (15 deg) then auto-detach in update()
  moveServoSmooth(ANGLE_CLOSED);
}

char DoorControl::scanAnalogColumnKeypad() {
  // 1. Baseline Idle Check: when ALL rows are HIGH (INPUT_PULLUP), A1 MUST be > 910.
  //    If A1 is < 910 while no row is pulled LOW, it is electrical noise or a short, NOT a key press.
  char detected = 0;
  for (byte r = 0; r < ROWS; r++) {
    // Pull only row 'r' LOW
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(100);

    int adc1 = analogRead(KEYPAD_COL_PIN);
    delayMicroseconds(40);
    int adc2 = analogRead(KEYPAD_COL_PIN);

    // Actively drive row HIGH briefly (helps D13 overcome onboard LED load) then return to INPUT_PULLUP
    digitalWrite(rowPins[r], HIGH);
    pinMode(rowPins[r], INPUT_PULLUP);

    // Verify A1 returns to HIGH (> 910) when row 'r' is released!

    // A real button press on row 'r' MUST:
    //  (a) have two consistent readings (abs(adc1 - adc2) < 20)
    //  (b) pull A1 below 860 while row 'r' is LOW
    //  (c) return above 910 immediately when row 'r' goes back HIGH
    if (abs(adc1 - adc2) < 20 && adc2 < 860) {
      int adc = (adc1 + adc2) / 2;
      byte col = 0;
      if (adc < 15) col = 0;       // Col 1 (0 ohm direct) -> ~0..180
      else if (adc < 40) col = 1;  // Col 2 (1k ohm)       -> ~450..550
      else if (adc < 80) col = 2;  // Col 3 (2.2k ohm)     -> ~640..730
      else col = 3;                 // Col 4 (4.7k ohm)     -> ~770..855
      detected = keys[r][col];
      break;
    }
  }

  // Non-blocking debounce state machine (requires 50ms stable state change)
  if (detected != lastAnalogKey) {
    lastKeyDebounce = millis();
    lastAnalogKey = detected;
  }

  if ((millis() - lastKeyDebounce) > 50) {
    if (detected != stableKey) {
      stableKey = detected;
      if (stableKey != 0) {
        return stableKey; // Trigger once per new key press
      }
    }
  }
  return 0;
}

void DoorControl::openDoor() {
  if (!isDoorOpen) {
    isDoorOpen = true;
    moveServoSmooth(ANGLE_OPEN);
    doorOpenTimer = millis();
  } else {
    doorOpenTimer = millis();
  }
}

void DoorControl::closeDoor() {
  if (isDoorOpen) {
    isDoorOpen = false;
    moveServoSmooth(ANGLE_CLOSED);
  }
}

bool DoorControl::verifyPassword(String inputPwd) {
  return inputPwd == password;
}

int DoorControl::getRemainingSeconds() {
  if (!isDoorOpen) return 0;
  unsigned long elapsed = millis() - doorOpenTimer;
  if (elapsed >= DOOR_OPEN_DURATION) return 0;
  return (int)((DOOR_OPEN_DURATION - elapsed + 999) / 1000);
}

void DoorControl::update() {
  // 1. Detach SG90 servo 600ms after movement finishes so it NEVER buzzes or drains 5V!
  if (isServoAttached && (millis() - servoAttachTimer >= SERVO_MOVE_TIME)) {
    myservo.detach();
    isServoAttached = false;
  }

  // 2. Auto close timer (20 seconds)
  if (isDoorOpen && (millis() - doorOpenTimer >= DOOR_OPEN_DURATION)) {
    closeDoor();
  }

  // 3. Read Keypad ('A' or '#' confirms password, 'C' or '*' clears)
  char key = scanAnalogColumnKeypad();
  if (key) {
    lastKey = key;
    if (key == 'A' || key == '#') {
      if (typed == password) {
        openDoor();
      }
      typed = "";
    }
    else if (key == 'C' || key == '*') {
      typed = "";
    }
    else if (key == 'D') {
      closeDoor();
    }
    else {
      if (typed.length() < 8) {
        typed += key;
      }
    }
    if (typed.length() >= 4) {
      if (typed == password) {
        openDoor();
      }
      typed = "";
    }
  }
}

bool DoorControl::getIsOpen() {
  return isDoorOpen;
}
