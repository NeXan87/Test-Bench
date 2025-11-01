#include "app_state.h"
#include "relays.h"
#include "config.h"

// =========================================================
// --- Внутреннее состояние ---
namespace {
Mode s_mode = MODE_MANUAL_BLOCKING;
bool s_groupA = true;
unsigned long s_relay1 = 1000, s_delay1 = 1000, s_relay2 = 1000, s_delay2 = 1000;
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
  pinMode(MODE0_SWITCH_PIN, INPUT_PULLUP);
  pinMode(MODE1_SWITCH_PIN, INPUT_PULLUP);
  pinMode(GROUP_SWITCH_PIN, INPUT_PULLUP);
}

// =========================================================
// --- Чтение переключателей режима/группы ---
bool app_state_readSwitches() {
  const bool m0 = !digitalRead(MODE0_SWITCH_PIN);
  const bool m1 = !digitalRead(MODE1_SWITCH_PIN);

  // Комбинации двух переключателей (2 бита)
  switch ((m1 << 1) | m0) {
    case 0b00: s_mode = MODE_MANUAL_BLOCKING; break;
    case 0b01: s_mode = MODE_SYNC_AUTO; break;
    case 0b10: s_mode = MODE_ASYNC_AUTO; break;
    case 0b11: s_mode = MODE_MANUAL_INDEPENDENT; break;
  }

  s_groupA = !digitalRead(GROUP_SWITCH_PIN);  // LOW = группа A
  return true;
}

// =========================================================
// --- Обновление состояний потенциометров с антидребезгом ---
void app_state_update() {
  const bool minDelayRequired = (!s_groupA) || (s_mode == MODE_ASYNC_AUTO);

  // 1️⃣ Считываем "сырые" значения АЦП
  int adc_on1 = analogRead(POT_ON1_PIN);
  int adc_d1 = analogRead(POT_DELAY1_PIN);
  int adc_on2 = analogRead(POT_ON2_PIN);
  int adc_d2 = analogRead(POT_DELAY2_PIN);
  int adc_cycles = analogRead(POT_CYCLES_PIN);

  // 2️⃣ Сглаживаем (EMA)
  f_on1 = smooth(f_on1, adc_on1);
  f_d1 = smooth(f_d1, adc_d1);
  f_on2 = smooth(f_on2, adc_on2);
  f_d2 = smooth(f_d2, adc_d2);
  f_cycles = smooth(f_cycles, adc_cycles);

  // 3️⃣ Пересчитываем в миллисекунды
  unsigned long raw_relay1 = mapFast((int)f_on1, 0, 1023, MIN_ON_RELAY1_TIME, MAX_ON_RELAY1_TIME) * 1000;
  unsigned long raw_relay2 = 0, raw_delay1 = 0, raw_delay2 = 0;

  if (s_mode == MODE_SYNC_AUTO) {
    raw_relay2 = mapFast((int)f_on2, 0, 1023, MIN_ON_RELAY2_TIME_SYNC, MAX_ON_RELAY2_TIME) * 1000;
  } else {
    raw_relay2 = mapFast((int)f_on2, 0, 1023, MIN_ON_RELAY2_TIME_ASYNC, MAX_ON_RELAY2_TIME) * 1000;
  }

  if (minDelayRequired) {
    raw_delay1 = mapFast((int)f_d1, 0, 1023, MIN_DELAY_TIME_GROUP_B, MAX_DELAY_TIME) * 1000;
    raw_delay2 = mapFast((int)f_d2, 0, 1023, MIN_DELAY_TIME_GROUP_B, MAX_DELAY_TIME) * 1000;
  } else {
    raw_delay1 = mapFast((int)f_d1, 0, 1023, MIN_DELAY_TIME_GROUP_A, MAX_DELAY_TIME) * 1000;
    raw_delay2 = mapFast((int)f_d2, 0, 1023, MIN_DELAY_TIME_GROUP_A, MAX_DELAY_TIME) * 1000;
  }

  // 4️⃣ Применяем ограничения и режимы
  s_relay1 = raw_relay1;
  s_delay1 = raw_relay2 < 1000 ? 1000 : raw_delay1;

  if (s_mode == MODE_SYNC_AUTO) {
    s_relay2 = raw_relay2 < 1000 ? 0 : raw_relay2;
    s_delay2 = raw_relay2 < 1000 ? 0 : raw_delay2;
  } else {
    s_relay2 = raw_relay2;
    s_delay2 = raw_delay2;
  }

  // 5️⃣ Потенциометр циклов
  int potCycles = constrain((int)f_cycles, 0, 1023);
  if (potCycles > INFINITY_THRESHOLD) {
    s_inf = true;
    s_cycles = MAX_CYCLES;
  } else {
    s_inf = false;

    if (potCycles == 0) {
      s_cycles = 1;  // только при самом левом положении
    } else {
      // Нелинейное преобразование: x^power
      float normalized = potCycles / 1023.0f;             // 0–1
      float curved = pow(normalized, CYCLE_CURVE_POWER);  // 0–1, но с искажением

      // Преобразуем в диапазон 1–999
      int rawCycles = (int)(curved * (MAX_CYCLES - MIN_CYCLES) + MIN_CYCLES);

      // Применяем многоступенчатый шаг
      if (rawCycles <= STEP1_MAX) {
        s_cycles = ((rawCycles - MIN_CYCLES) / STEP1_SIZE) * STEP1_SIZE + MIN_CYCLES;
      } else if (rawCycles <= STEP2_MAX) {
        s_cycles = ((rawCycles - STEP1_MAX - 1) / STEP2_SIZE) * STEP2_SIZE + STEP1_MAX;
      } else if (rawCycles <= STEP3_MAX) {
        s_cycles = ((rawCycles - STEP2_MAX - 1) / STEP3_SIZE) * STEP3_SIZE + STEP2_MAX;
      } else {
        s_cycles = ((rawCycles - STEP3_MAX - 1) / STEP4_SIZE) * STEP4_SIZE + STEP3_MAX;
      }

      // Убедимся, что не превышает максимум
      if (s_cycles > MAX_CYCLES) s_cycles = MAX_CYCLES;
    }
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
  return s_relay1;
}
unsigned long app_state_getDelay1Time() {
  return s_delay1;
}
unsigned long app_state_getRelay2Time() {
  return s_relay2;
}
unsigned long app_state_getDelay2Time() {
  return s_delay2;
}
int app_state_getCycleLimit() {
  return s_cycles;
}
bool app_state_getInfiniteCycles() {
  return s_inf;
}