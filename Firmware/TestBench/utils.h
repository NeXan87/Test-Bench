#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

const char* utils_formatTimeSec(unsigned long ms);
const char* utils_formatCycleTime(unsigned long elapsedMs);

#endif