#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>
#include <EEPROM.h>

void calibration_run();
void calibration_load();
void calibration_save();
int calibration_getCalibratedADC(int raw, uint8_t potIndex);
bool calibration_isSaving();

#endif