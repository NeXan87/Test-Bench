#include "current.h"
#include "config.h"
#include <ACS712.h>  // Библиотека RobTillaart

ACS712 sensor(CURRENT_SENSOR_PIN, 5.0, 1024, M_VPER_AMPERE);

float current_readDC() {
  float current = sensor.mA_DC() / 1000.0f;  // в амперах
  current = (int)(current * 10.0f + 0.5f) / 10.0f;
  if (current > MAX_CURRENT) current = MAX_CURRENT + 0.5f;      // ограничиваем 30А
  return current;
}