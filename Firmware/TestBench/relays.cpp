#include "relays.h"

// --- Состояние выбранной группы ---
static uint8_t s_r1, s_r2;

// --- Массив всех реле ---
static const uint8_t RELAY_PINS[] = {
  RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN
};
constexpr uint8_t RELAY_COUNT = sizeof(RELAY_PINS);

void relays_init() {
  for (uint8_t i = 0; i < RELAY_COUNT; ++i) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);
  }
}

void relays_setGroup(RelayGroup group) {
  if (group == GROUP_A) {
    s_r1 = RELAY1_PIN;
    s_r2 = RELAY2_PIN;
  } else {
    s_r1 = RELAY3_PIN;
    s_r2 = RELAY4_PIN;
  }
}

void relays_activateFirst(bool blockOther) {
  if (blockOther) digitalWrite(s_r2, LOW);
  digitalWrite(s_r1, HIGH);
}

void relays_activateSecond(bool blockOther) {
  if (blockOther) digitalWrite(s_r1, LOW);
  digitalWrite(s_r2, HIGH);
}

void relays_deactivateAll() {
  for (uint8_t i = 0; i < RELAY_COUNT; ++i)
    digitalWrite(RELAY_PINS[i], LOW);
}

bool relays_isIdle() {
  for (uint8_t i = 0; i < RELAY_COUNT; ++i)
    if (digitalRead(RELAY_PINS[i]) == HIGH) return false;
  return true;
}