#ifndef CURRENT_H
#define CURRENT_H

#include <Arduino.h>

float current_readDC();
void current_updateOverloadProtection(float current);
bool current_isOverload();
void current_resetOverload();
void relays_deactivateAll();
void ui_clearLEDs();

#endif