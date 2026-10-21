#include "Max7219Matrix.h"

Max7219Matrix::Max7219Matrix(int dPin, int cPin, int cs) {
  dinPin = dPin;
  clkPin = cPin;
  csPin  = cs;
  for (int i = 0; i < 8; i++) {
    indoorRows[i] = 0x00;
    outdoorRows[i] = 0x00;
  }
}

void Max7219Matrix::sendCommand(uint8_t reg, uint8_t dataIndoor, uint8_t dataOutdoor) {
  // 1. قلب بن 13 لـ OUTPUT قوي أثناء إرسال بيانات الشاشة
  pinMode(clkPin, OUTPUT);

  digitalWrite(csPin, LOW);
  shiftOut(dinPin, clkPin, MSBFIRST, reg);
  shiftOut(dinPin, clkPin, MSBFIRST, dataOutdoor); // موديول 2 الخارجي
  shiftOut(dinPin, clkPin, MSBFIRST, reg);
  shiftOut(dinPin, clkPin, MSBFIRST, dataIndoor);  // موديول 1 الداخلي
  digitalWrite(csPin, HIGH);

  // 2. إرجاع بن 13 لـ INPUT_PULLUP فوراً عشان الكيباد تقرأ الصف الأول R1 بدون أي مشاكل
  pinMode(clkPin, INPUT_PULLUP);
}

void Max7219Matrix::begin() {
  pinMode(dinPin, OUTPUT);
  pinMode(clkPin, OUTPUT);
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);

  sendCommand(0x09, 0x00, 0x00); // Decode mode: raw
  sendCommand(0x0B, 0x07, 0x07); // مسح الصفوف
  sendCommand(0x0F, 0x00, 0x00); // إطفاء التيست
  sendCommand(0x0C, 0x01, 0x01); // تفعيل التشغيل
  sendCommand(0x0A, 0x08, 0x08); // شدة الإضاءة
  clear();
}

void Max7219Matrix::setIndoorLight(bool on) {
  uint8_t val = on ? 0xFF : 0x00;
  for (int r = 0; r < 8; r++) {
    indoorRows[r] = val;
    sendCommand(r + 1, indoorRows[r], outdoorRows[r]);
  }
}

void Max7219Matrix::setOutdoorLight(bool on) {
  uint8_t val = on ? 0xFF : 0x00;
  for (int r = 0; r < 8; r++) {
    outdoorRows[r] = val;
    sendCommand(r + 1, indoorRows[r], outdoorRows[r]);
  }
}

void Max7219Matrix::setBrightness(int level) {
  uint8_t intensity = constrain(level, 0, 15);
  sendCommand(0x0A, intensity, intensity);
}

void Max7219Matrix::clear() {
  for (int r = 0; r < 8; r++) {
    indoorRows[r] = 0x00;
    outdoorRows[r] = 0x00;
    sendCommand(r + 1, 0x00, 0x00);
  }
}

// دوال الربط العامة
void setIndoorLight(bool on) {
  matrix.setIndoorLight(on);
}

void setOutdoorLight(bool on) {
  matrix.setOutdoorLight(on);
}