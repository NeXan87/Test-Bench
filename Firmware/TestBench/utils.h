#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

const char* utils_formatTimeSec(unsigned long ms);
const char* utils_formatCycleTime(unsigned long elapsedMs, char* buffer, size_t size);
void utils_setChars(uint8_t row, uint8_t col, int value, bool isClean = false);

#endif