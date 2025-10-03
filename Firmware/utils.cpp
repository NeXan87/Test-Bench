#include "utils.h"

const char* utils_formatTimeSec(unsigned long ms) {
  static char buffer[5]; // "00s\0"
  int sec = ms / 1000;
  sprintf(buffer, "%02ds", sec);
  return buffer;
}

const char* utils_formatCycleTime(unsigned long elapsedMs) {
  static char buffer[7]; // "MM:SS"
  
  // Максимум 99 минут 59 секунд = 5999 секунд
  unsigned long totalSec = (elapsedMs / 1000) % 6000; // 6000 = 100*60
  
  byte minutes = totalSec / 60;
  byte seconds = totalSec % 60;
  
  sprintf(buffer, "%02d:%02d", minutes, seconds);
  return buffer;
}