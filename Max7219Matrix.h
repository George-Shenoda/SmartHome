#ifndef MAX7219MATRIX_H
#define MAX7219MATRIX_H

#include <Arduino.h>

class Max7219Matrix {
  private:
    int dinPin;
    int clkPin;
    int csPin;
    uint8_t indoorRows[8];
    uint8_t outdoorRows[8];

    void sendCommand(uint8_t reg, uint8_t dataIndoor, uint8_t dataOutdoor);

  public:
    Max7219Matrix(int dPin, int cPin, int cs);
    void begin();
    void setIndoorLight(bool on);
    void setOutdoorLight(bool on);
    void setBrightness(int level); // من 0 إلى 15
    void clear();
};

// تعريف كائن عام يمكن الوصول إليه من أي ملف
extern Max7219Matrix matrix;

// دوال مساعدة للتوافق المباشر
void setIndoorLight(bool on);
void setOutdoorLight(bool on);

#endif