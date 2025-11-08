#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init();
void display_diagnosctic_init();
void display_clear();
void display_showDiagnostic();
void display_update(Mode mode, bool groupA, float current);

#endif