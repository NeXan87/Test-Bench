#include "current.h"
#include "config.h"
#include <ACS712.h>  // Библиотека RobTillaart

ACS712 sensor(CURRENT_SENSOR_PIN, 5.0, 1024, M_VPER_AMPERE);

namespace {
bool s_overloadDetected = false;
unsigned long s_overloadStartTime = 0;
bool g_isOverload = false;
}

float current_readDC() {
  return sensor.mA_DC(1) / 1000.0f;  // в амперах
}

void current_updateOverloadProtection(float current) {
  if (current > OVERLOAD_CURRENT_LIMIT) {
    if (!s_overloadDetected) {
      s_overloadDetected = true;
      s_overloadStartTime = millis();
    } else {
      if (millis() - s_overloadStartTime >= OVERLOAD_TIME) {
        g_isOverload = true;
        relays_deactivateAll();
        ui_clearLEDs();
      }
    }
  } else {
    s_overloadDetected = false;
  }
}

bool current_isOverload() {
  return g_isOverload;
}

void current_resetOverload() {
  s_overloadDetected = false;
  g_isOverload = false;
}