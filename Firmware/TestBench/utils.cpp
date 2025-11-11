#include "utils.h"
#include "config.h"
#include "display.h"

const char* utils_formatTimeSec(unsigned long ms) {
  static char buffer[4];     // "000"
  uint16_t sec = ms / 1000;  // для скорости — int заменен на uint16_t
  buffer[0] = '0' + (sec / 10) % 10;
  buffer[1] = '0' + sec % 10;
  buffer[2] = '\0';
  return buffer;
}

const char* utils_formatCycleTime(unsigned long elapsedMs, char* buffer, size_t size) {
  if (size < 9) return;  // защита от переполнения

  unsigned long totalSec = (elapsedMs / 1000) % (MAX_DISPLAY_TIME_HH * 3600 + MAX_DISPLAY_TIME_MM * 60 + MAX_DISPLAY_TIME_SS + 1);

  uint8_t hours = totalSec / 3600;
  uint8_t minutes = (totalSec % 3600) / 60;
  uint8_t seconds = totalSec % 60;

  buffer[0] = '0' + (hours / 10) % 10;
  buffer[1] = '0' + hours % 10;
  buffer[2] = ':';
  buffer[3] = '0' + (minutes / 10) % 10;
  buffer[4] = '0' + minutes % 10;
  buffer[5] = ':';
  buffer[6] = '0' + (seconds / 10) % 10;
  buffer[7] = '0' + seconds % 10;
  buffer[8] = '\0';

  return buffer;
}

void utils_setChars(uint8_t col, uint8_t row, int value, bool isClean = false) {
  lcd.setCursor(col, row);
  if (isClean) {
    lcd.print(F("    "));      // очистить 4 символа
    lcd.setCursor(col, row);   // вернуть курсор
  }
  lcd.print(value);
}