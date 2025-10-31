#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init();
void display_clear();
void display_update(
  unsigned long t1, unsigned long d1,
  unsigned long t2, unsigned long d2,
  int currentCycle,
  int totalCycles,
  bool infinite,
  uint8_t mode,
  bool groupA,
  float current
);

#endif