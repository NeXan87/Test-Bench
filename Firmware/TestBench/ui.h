#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <Bounce2.h>

void ui_init();
void ui_runStartupAnimation();
void ui_updateButtons();
void ui_clearLEDs();
void ui_blinkAllLEDs();
bool ui_start1Pressed();
bool ui_start2Pressed();
bool ui_start2Held();
bool ui_isStopHeld();
void ui_updateLEDs(bool r1Active, bool r1On, bool r2Active, bool r2On, uint8_t mode);

#endif