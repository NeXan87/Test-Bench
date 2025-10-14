#include "app_state.h"
#include "relays.h"
#include "config.h"

// =========================================================
// --- Внутреннее состояние ---
namespace {
Mode s_mode = MODE_MANUAL_BLOCKING;
bool s_groupA = true;
unsigned long s_t1 = 1000, s_d1 = 1000, s_t2 = 1000, s_d2 = 1000;
int s_cycles = 1;
bool s_inf = false;
int s_currentCycle = 0;

// Храним фильтрованные значения АЦП
float f_on1 = 512, f_on2 = 512, f_d1 = 512, f_d2 = 512, f_cycles = 512;

// --- Вспомогательные функции ---
inline unsigned long ensureMin(unsigned long val, unsigned long minVal) {
  return (val < minVal) ? minVal : val;
}

inline float smooth(float prev, int raw, float alpha = 0.7f) {
  return prev * alpha + raw * (1.0f - alpha);
}

// Быстрый inline map
inline unsigned long mapFast(int x, int in_min, int in_max, unsigned long out_min, unsigned long out_max) {
  return (unsigned long)((x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min);
}
}

// =========================================================
// --- Инициализация ---
void app_state_init() {
  pinMode(MODE0_PIN, INPUT_PULLUP);
  pinMode(MODE1_PIN, INPUT_PULLUP);
  pinMode(GROUP_PIN, INPUT_PULLUP);
}

// =========================================================
// --- Чтение переключателей режима/группы ---
bool app_state_readSwitches() {
  const bool m0 = !digitalRead(MODE0_PIN);
  const bool m1 = !digitalRead(MODE1_PIN);

  // Комбинации двух переключателей (2 бита)
  switch ((m1 << 1) | m0) {
    case 0b00: s_mode = MODE_MANUAL_BLOCKING; break;
    case 0b01: s_mode = MODE_SYNC_AUTO; break;
    case 0b10: s_mode = MODE_ASYNC_AUTO; break;
    case 0b11: s_mode = MODE_MANUAL_INDEPENDENT; break;
  }

  s_groupA = !digitalRead(GROUP_PIN);  // LOW = группа A
  return true;
}

// =========================================================
// --- Обновление состояний потенциометров с антидребезгом ---
void app_state_update() {
  // 1️⃣ Считываем "сырые" значения АЦП
  int adc_on1    = analogRead(POT_ON1_PIN);
  int adc_d1     = analogRead(POT_DELAY1_PIN);
  int adc_on2    = analogRead(POT_ON2_PIN);
  int adc_d2     = analogRead(POT_DELAY2_PIN);
  int adc_cycles = analogRead(POT_CYCLES_PIN);

  // 2️⃣ Сглаживаем (EMA)
  f_on1    = smooth(f_on1, adc_on1);
  f_d1     = smooth(f_d1, adc_d1);
  f_on2    = smooth(f_on2, adc_on2);
  f_d2     = smooth(f_d2, adc_d2);
  f_cycles = smooth(f_cycles, adc_cycles);

  // 3️⃣ Пересчитываем в миллисекунды
  unsigned long raw_t1 = mapFast((int)f_on1, 0, 1023, MIN_ON_TIME, MAX_ON_TIME);
  unsigned long raw_t2 = mapFast((int)f_on2, 0, 1023, MIN_ON_TIME, MAX_ON_TIME);
  unsigned long raw_delay1 = mapFast((int)f_d1, 0, 1023, MIN_DELAY_TIME, MAX_DELAY_TIME);
  unsigned long raw_delay2 = mapFast((int)f_d2, 0, 1023, MIN_DELAY_TIME, MAX_DELAY_TIME);

  // 4️⃣ Применяем ограничения и режимы
  if (s_mode == MODE_SYNC_AUTO) {
    s_t1 = ensureMin(raw_t1, 1000);
    s_t2 = raw_t2;
  } else {
    const bool allowZeroT2 = (!s_groupA) || (s_mode == MODE_MANUAL_INDEPENDENT);
    s_t1 = ensureMin(raw_t1, 1000);
    s_t2 = allowZeroT2 ? raw_t2 : ensureMin(raw_t2, 1000);
  }

  const bool minDelayRequired = (!s_groupA) || (s_mode == MODE_ASYNC_AUTO);
  if (minDelayRequired) {
    s_d1 = ensureMin(raw_delay1, 1000);
    s_d2 = ensureMin(raw_delay2, 1000);
  } else {
    s_d1 = raw_delay1;
    s_d2 = raw_delay2;
  }

  // 5️⃣ Потенциометр циклов
  int potCycles = constrain((int)f_cycles, 0, 1023);
  if (potCycles > INFINITY_THRESHOLD) {
    s_inf = true;
    s_cycles = INFINITY_THRESHOLD;
  } else {
    s_inf = false;
    s_cycles = mapFast(potCycles, 0, 1000, MIN_CYCLES, MAX_CYCLES);
  }
}

// =========================================================
// --- Геттеры / Сеттеры ---
void app_state_setCurrentCycle(int cycle) {
  s_currentCycle = cycle;
}
int app_state_getCurrentCycle() {
  return s_currentCycle;
}
Mode app_state_getMode() {
  return s_mode;
}
bool app_state_getGroupA() {
  return s_groupA;
}
unsigned long app_state_getRelay1Time() {
  return s_t1;
}
unsigned long app_state_getDelay1Time() {
  return s_d1;
}
unsigned long app_state_getRelay2Time() {
  return s_t2;
}
unsigned long app_state_getDelay2Time() {
  return s_d2;
}
int app_state_getCycleLimit() {
  return s_cycles;
}
bool app_state_getInfiniteCycles() {
  return s_inf;
}
bool app_state_isIdle() {
  return relays_isIdle();
}