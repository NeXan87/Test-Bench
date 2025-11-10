#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "app_state.h"

void display_init(bool isDiagnosticMode, bool g_isCalibrateMode);
void display_clear();
void display_showDiagnostic();
void display_update(Mode mode, bool groupA, float current);

extern LiquidCrystal_I2C lcd; 

#endif