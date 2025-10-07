#include "ui.h"
#include "config.h"

Bounce g_btn1;
Bounce g_btn2;
Bounce g_btnStop;

void ui_init() {
  pinMode(START1_PIN, INPUT_PULLUP);
  pinMode(START2_PIN, INPUT_PULLUP);
  pinMode(STOP_PIN, INPUT_PULLUP);
  
  g_btn1.attach(START1_PIN);
  g_btn1.interval(25);
  g_btn2.attach(START2_PIN);
  g_btn2.interval(25);
  g_btnStop.attach(STOP_PIN);
  g_btnStop.interval(25);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
}

void ui_runStartupAnimation() {
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED3_PIN, HIGH);
  digitalWrite(LED4_PIN, HIGH);
  
}

void ui_updateButtons() {
  g_btn1.update();
  g_btn2.update();
  g_btnStop.update();
}

void ui_clearLEDs() {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
}

bool ui_start1Pressed() { return g_btn1.fell(); }
bool ui_start2Pressed() { return g_btn2.fell(); }

bool ui_isStopHeld() {
  return !g_btnStop.read();
}

// bool ui_stopReleased() {
//   static bool wasHeld = false;
//   bool isHeld = ui_isStopHeld();
//   bool released = wasHeld && !isHeld;
//   wasHeld = isHeld;
//   return released;
// }

void ui_updateLEDs(bool r1Active, bool r1On, bool r2Active, bool r2On, int mode) {
  if (mode == 3) {
    digitalWrite(LED1_PIN, r1Active && r1On);
    digitalWrite(LED2_PIN, r1Active && !r1On);
    digitalWrite(LED3_PIN, r2Active && r2On);
    digitalWrite(LED4_PIN, r2Active && !r2On);
  } else {
    digitalWrite(LED1_PIN, r1On);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, r2On);
    digitalWrite(LED4_PIN, LOW);
  }
}

void ui_blinkAllLEDs() {
  static unsigned long lastToggle = 0;
  const unsigned long interval = BLINK_INTERVAL_MS;
  unsigned long now = millis();

  if (now - lastToggle >= interval) {
    bool state = (now / interval) % 2;
    digitalWrite(LED1_PIN, state);
    digitalWrite(LED2_PIN, state);
    digitalWrite(LED3_PIN, state);
    digitalWrite(LED4_PIN, state);
    lastToggle = now;
  }
}