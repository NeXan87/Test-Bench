#include "ui.h"
#include "config.h"

Bounce g_btn1;
Bounce g_btn2;
Bounce g_btnStop;

// --- Пины в массивах для компактности ---
const uint8_t LED_PINS[] = { LED_ON1_PIN, LED2_DELAY1_PIN, LED_ON2_PIN, LED4_DELAY2_PIN };
constexpr uint8_t LED_COUNT = sizeof(LED_PINS);

// ==========================
// ИНИЦИАЛИЗАЦИЯ
// ==========================

void ui_init() {
  // Кнопки
  pinMode(START1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(START2_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

  g_btn1.attach(START1_BUTTON_PIN, INPUT_PULLUP);
  g_btn2.attach(START2_BUTTON_PIN, INPUT_PULLUP);
  g_btnStop.attach(STOP_BUTTON_PIN, INPUT_PULLUP);

  g_btn1.interval(25);
  g_btn2.interval(25);
  g_btnStop.interval(25);

  // Светодиоды
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);  // сразу гасим
  }
}

// ==========================
// АНИМАЦИЯ ЗАПУСКА
// ==========================

void ui_runStartupAnimation() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);
  }
}

// ==========================
// КНОПКИ
// ==========================

void ui_updateButtons() {
  g_btn1.update();
  g_btn2.update();
  g_btnStop.update();
}

bool ui_start1Pressed() {
  return g_btn1.fell();
}
bool ui_start2Pressed() {
  return g_btn2.fell();
}
bool ui_start2Held() {
  return !g_btn2.read();
}
bool ui_isStopHeld() {
  return !g_btnStop.read();
}

// ==========================
// СВЕТОДИОДЫ
// ==========================

inline void ui_setLED(uint8_t index, bool state) {
  if (index < LED_COUNT) {
    digitalWrite(LED_PINS[index], state ? HIGH : LOW);
  }
}

void ui_clearLEDs() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void ui_updateLEDs(bool r1Active, bool r1On, bool r2Active, bool r2On, uint8_t mode) {
  if (mode == 3) {
    ui_setLED(0, r1Active && r1On);
    ui_setLED(1, r1Active && !r1On);
    ui_setLED(2, r2Active && r2On);
    ui_setLED(3, r2Active && !r2On);
  } else {
    ui_setLED(0, r1On);
    ui_setLED(1, LOW);
    ui_setLED(2, r2On);
    ui_setLED(3, LOW);
  }
}

void ui_blinkAllLEDs() {
  static unsigned long lastToggle = 0;
  const unsigned long interval = BLINK_LED_INTERVAL;
  unsigned long now = millis();

  if (now - lastToggle >= interval) {
    bool state = ((now / interval) & 1);
    for (uint8_t i = 0; i < LED_COUNT; i++)
      ui_setLED(i, state);
    lastToggle = now;
  }
}