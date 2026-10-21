#include <Wire.h>
#include <dht.h>

#include "Light.h"
#include "ClimateControl.h"
#include "DoorControl.h"
#include "DisplayManager.h"
#include "BluetoothManager.h"
#include "SecuritySystem.h"
#include "Max7219Matrix.h"

// ==========================================
// CLASS OBJECTS
// ==========================================
// Function 3 — Indoor Lighting: Switch #1 = D3, LDR = A0, Potentiometer #1 = A6
Light light = Light(3, A0, A6);

// Function 4 — Climate Control: DHT11 = D4, Fan Motor = D9, Potentiometer #2 = A7
ClimateControl climate = ClimateControl(4, 9, A7);

// Function 1 — Main Door Access: Servo = D2, Password = "2006"
DoorControl door = DoorControl(2, "2006");

// Function 5 — LCD Feedback Display: I2C Address 0x27 (16x2) on A4 (SDA) & A5 (SCL)
DisplayManager display = DisplayManager(0x27, 16, 2);

// Wireless Link — Bluetooth Classic SPP on D0 (RX) & D1 (TX)
BluetoothManager bluetooth;

// Function 2 — Security System: Switch #2 = D8, Window IR2 = A2, Door PIR = A3, Buzzer = D5
SecuritySystem security = SecuritySystem(8, A2, A3, 5);

dht DHT;

// إنشاء الكائن (DIN=6, CLK=13, CS=7)
Max7219Matrix matrix(6, 13, 7);

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================
bool isSecurityArmed   = true;
bool isWindowOpen      = false;
bool isMotionDetected  = false;
bool isFanOn           = false;
float currentTemp      = 0.0;
float currentHum       = 0.0;
float tempThreshold    = 28.0;
char lastKey = "";

unsigned long lastUpdateMillis = 0;

void setup() {
  pinMode(light.lightSwitch, INPUT_PULLUP);
  pinMode(light.LDR, INPUT);

  Wire.begin();
  Wire.setClock(100000);

  bluetooth.begin(9600);
  display.begin();

  light.begin();
  climate.begin();
  door.begin();
  security.begin();
  matrix.begin();

  display.update(door.getIsOpen(), isSecurityArmed, currentTemp, currentHum, lastKey);
}

void loop() {
  light.LDRCheck();
  light.toggleLightBtn();
  climate.update();
  door.update();
  security.update();
  bluetooth.readCommand();

  if (millis() - lastUpdateMillis >= 500) {
    lastUpdateMillis = millis();
    display.update(door.getIsOpen(), isSecurityArmed, currentTemp, currentHum, lastKey);
    bluetooth.sendStatus();
  }
}