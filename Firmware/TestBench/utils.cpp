#include "utils.h"
#include "config.h"

const char* utils_formatTimeSec(unsigned long ms) {
  static char buffer[4]; // "00"
  uint16_t sec = ms / 1000; // для скорости — int заменен на uint16_t
  buffer[0] = '0' + (sec / 10) % 10;
  buffer[1] = '0' + sec % 10;
  buffer[2] = '\0';
  return buffer;
}

const char* utils_formatCycleTime(unsigned long elapsedMs) {
  static char buffer[6]; // "MM:SS" + '\0'
  uint16_t totalSec = (elapsedMs / 1000UL) % MAX_DISPLAY_TIME_SEC; // безопасный тип
  uint8_t minutes = totalSec / 60;
  uint8_t seconds = totalSec % 60;

  // Быстрее, чем sprintf (экономия > 1 КБ флеша на AVR)
  buffer[0] = '0' + (minutes / 10);
  buffer[1] = '0' + (minutes % 10);
  buffer[2] = ':';
  buffer[3] = '0' + (seconds / 10);
  buffer[4] = '0' + (seconds % 10);
  buffer[5] = '\0';
  
  return buffer;
}