#include "relays.h"

static int s_r1, s_r2;

void relays_init() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  relays_deactivateAll();
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
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);
  digitalWrite(RELAY4_PIN, LOW);
}

bool relays_isIdle() {
  return digitalRead(RELAY1_PIN) == LOW &&
         digitalRead(RELAY2_PIN) == LOW &&
         digitalRead(RELAY3_PIN) == LOW &&
         digitalRead(RELAY4_PIN) == LOW;
}