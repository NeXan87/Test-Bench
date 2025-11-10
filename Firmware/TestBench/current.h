#ifndef CURRENT_H
#define CURRENT_H

#include <Arduino.h>

void current_setMidPoint();
float current_readDC();
void current_updateOverloadProtection(float current);
bool current_isOverload();
void current_resetOverload();

#endif