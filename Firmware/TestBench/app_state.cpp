#include "app_state.h"
#include "relays.h"
#include "config.h"

namespace {
Mode s_mode = MODE_MANUAL_BLOCKING;
bool s_groupA = true;
unsigned long s_t1 = 1000, s_d1 = 1000, s_t2 = 1000, s_d2 = 1000;
int s_cycles = 1;
bool s_inf = false;
int s_currentCycle = 0;
}

void app_state_init() {
  pinMode(MODE0_PIN, INPUT_PULLUP);
  pinMode(MODE1_PIN, INPUT_PULLUP);
  pinMode(GROUP_PIN, INPUT_PULLUP);
}

bool app_state_readSwitches() {
  bool m0 = !digitalRead(MODE0_PIN);  // D4
  bool m1 = !digitalRead(MODE1_PIN);  // D5

  if (!m0 && !m1) {
    s_mode = MODE_MANUAL_BLOCKING;
  } else if (m0 && !m1) {
    s_mode = MODE_SYNC_AUTO;
  } else if (!m0 && m1) {
    s_mode = MODE_ASYNC_AUTO;
  } else if (m0 && m1) {
    s_mode = MODE_MANUAL_INDEPENDENT;  // Режим 4
  }

  s_groupA = !digitalRead(GROUP_PIN);  // D6: LOW = группа A
  return true;
}

void app_state_update() {
  // Время включения реле: 0–60 сек
  unsigned long raw_t1 = map(analogRead(POT_ON1_PIN), 0, 1023, MIN_ON_TIME, MAX_ON_TIME);
  unsigned long raw_t2 = map(analogRead(POT_ON2_PIN), 0, 1023, MIN_ON_TIME, MAX_ON_TIME);

  // Задержка: сначала читаем "сырые" значения
  unsigned long raw_d1 = map(analogRead(POT_DELAY1_PIN), 0, 1023, MIN_DELAY_TIME, MAX_DELAY_TIME);
  unsigned long raw_d2 = map(analogRead(POT_DELAY2_PIN), 0, 1023, MIN_DELAY_TIME, MAX_DELAY_TIME);

  if (s_mode == MODE_SYNC_AUTO) {
    s_t1 = (raw_t1 < 1000) ? 1000 : raw_t1;  // 1–60 сек
    s_t2 = raw_t2;                           // 0–60 сек
  } else {
    // Во всех остальных режимах:
    // - Реле 1/3 → 1–60 сек
    // - Реле 2/4 → 1–60 сек (или 0 сек в зависимости от группы/режима)
    s_t1 = (raw_t1 < 1000) ? 1000 : raw_t1;

    // Для реле 2/4: 0 сек разрешено только в группе B или режиме 4
    bool allowZeroT2 = (!s_groupA) || (s_mode == MODE_MANUAL_INDEPENDENT);
    s_t2 = allowZeroT2 ? raw_t2 : ((raw_t2 < 1000) ? 1000 : raw_t2);
  }

  // Минимальная задержка 1 сек ТОЛЬКО если требуется (группа B или режим 4)
  bool minDelayRequired = (!s_groupA) || (s_mode == MODE_ASYNC_AUTO);
  if (minDelayRequired) {
    s_d1 = (raw_d1 < 1000) ? 1000 : raw_d1;
    s_d2 = (raw_d2 < 1000) ? 1000 : raw_d2;
  } else {
    s_d1 = raw_d1;
    s_d2 = raw_d2;
  }

  // Потенциометр циклов
  int potCycles = analogRead(POT_CYCLES_PIN);
  potCycles = constrain(potCycles, 0, 1023);

  if (potCycles > INFINITY_THRESHOLD) {
    s_inf = true;
    s_cycles = INFINITY_THRESHOLD;
  } else {
    s_inf = false;
    s_cycles = map(potCycles, 0, 1000, MIN_CYCLES, MAX_CYCLES);  // ← 0→1, 500→500, 1000→1000
  }
}

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