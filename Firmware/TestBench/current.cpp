#include "current.h"
#include "config.h"
#include <ACS712.h>  // Библиотека RobTillaart

ACS712 sensor(CURRENT_SENSOR_PIN, 5.0, 1024, M_VPER_AMPERE);

float current_readDC() {
  return sensor.mA_DC(1) / 1000.0f;  // в амперах
}