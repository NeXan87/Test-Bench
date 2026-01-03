#include "relays.h"

// --- Состояние выбранной группы ---
static uint8_t s_r1, s_r2;

// --- Массив всех реле ---
static const uint8_t RELAY_PINS[] = {
  RELAY1_24V_PIN, RELAY2_24V_PIN, RELAY3_380V_PIN, RELAY4_380V_PIN
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
    s_r1 = RELAY1_24V_PIN;
    s_r2 = RELAY2_24V_PIN;
  } else {
    s_r1 = RELAY3_380V_PIN;
    s_r2 = RELAY4_380V_PIN;
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