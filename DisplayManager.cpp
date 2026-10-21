#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t addr, uint8_t cols, uint8_t rows) : lcd(addr, cols, rows) {
}

void DisplayManager::begin() {
  lcd.init();
  lcd.backlight();
}

void DisplayManager::update(bool isDoorOpen, bool isSecArmed, float temp, float hum, char lastKey) {
  char line1[17];
  char line2[17];

  // تجهيز السطر الأول: 16 حرف بالظبط
  snprintf(line1, sizeof(line1), "D:%-4s S:%-3s K:%-1c", 
           isDoorOpen ? "OPND" : "CLSD", 
           isSecArmed ? "ARM" : "OFF",
           lastKey);

  // تجهيز السطر الثاني: الحرارة والرطوبة
  snprintf(line2, sizeof(line2), "T:%d.%dC  H:%d%%   ", 
           (int)temp, abs((int)(temp * 10) % 10), (int)hum);

  // تحديث الشاشة
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);
}