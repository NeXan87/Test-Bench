#include "current.h"
#include "config.h"
#include <Arduino.h>

float current_readDC() {
  static long sum = 0;
  static int count = 0;
  const int SAMPLES = 100;
  
  sum += analogRead(CURRENT_SENSOR_PIN);
  count++;
  
  if (count >= SAMPLES) {
    float avg = sum / (float)SAMPLES;
    sum = 0;
    count = 0;
    
    float voltage = (avg - ADC_ZERO_OFFSET) * (5.0f / 1023.0f);
    float current = voltage * CURRENT_SCALE;
    return current < 0 ? -current : current;
  }
  
  // Возвращаем последнее известное значение
  static float lastCurrent = 0.0f;
  return lastCurrent;
}