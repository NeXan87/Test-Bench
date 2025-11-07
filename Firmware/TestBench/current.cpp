#include "current.h"
#include "config.h"
#include "relays.h"
#include "ui.h"
#include <ACS712.h>  // Библиотека RobTillaart

ACS712 sensor(CURRENT_SENSOR_PIN, VOLTAGE_ACS712, 1024, M_VPER_AMPERE);

namespace {
bool s_overloadDetected = false;
unsigned long s_overloadStartTime = 0;
bool g_isOverload = false;
}

float current_readDC() {
  static float filtered = 0.0f;   // сохраняется между вызовами

  // Сырое значение в амперах
  float raw = fabs(sensor.mA_DC(CURRENT_SAMPLES) / 1000.0f);

  // Фильтр EMA (экспоненциальное скользящее среднее)
  filtered = filtered + CURRENT_ALPHA * (raw - filtered);

  // Округление до одного знака после запятой
  float rounded = round(filtered * 10.0f) / 10.0f;

  return rounded;
}

void current_updateOverloadProtection(float current) {
  if (current >= OVERLOAD_CURRENT_LIMIT) {
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