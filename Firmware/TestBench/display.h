#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init(bool isDiagnosticMode, bool g_isCalibrateMode);
void display_clear();
void display_showDiagnostic();
void display_showCalibrate();
void display_update(Mode mode, bool groupA, float current);

#endif