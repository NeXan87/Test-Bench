#include "current.h"
#include "config.h"
#include <Arduino.h>

float current_readDC() {
  // Усредняем 100 отсчётов для уменьшения шума
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(CURRENT_SENSOR_PIN);
    delay(1);
  }
  float avg = sum / 100.0;

  // Преобразуем в ток (A)
  // ACS712: 2.5V = 0A → 512 в ADC
  float voltage = (avg - 512.0) * (5.0 / 1023.0);
  float current = voltage * CURRENT_SCALE;

  // Возвращаем модуль (постоянный ток)
  return current < 0 ? -current : current;
}