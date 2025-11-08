#include "config.h"
#include "ui.h"
#include "relays.h"
#include "app_state.h"
#include "modes.h"
#include "display.h"
#include "current.h"

namespace {
float current = 0.0f;
Mode currMode = MODE_MANUAL_BLOCKING;
bool isGroupA = true;
bool g_isDiagnosticMode = false;
}

void setup() {
  // Инициализация кнопок ДО всего остального
  pinMode(START2_BUTTON_PIN, INPUT_PULLUP);

  // Проверка диагностики: удерживается ли Пуск2?
  delay(50);  // дать стабилизироваться
  if (digitalRead(START2_BUTTON_PIN) == LOW) {
    g_isDiagnosticMode = true;
  }

  ui_init();
  relays_init();
  app_state_init();
  modes_init();
  display_init();

  if (g_isDiagnosticMode) {
    
    while (digitalRead(START2_BUTTON_PIN) == LOW) {
      delay(10);  // ждём отпускания
    }
  } else {
    ui_runStartupAnimation();
    delay(STARTUP_TIMEOUT);
  }

  ui_clearLEDs();
  display_clear();
}

void loop() {
  if (g_isDiagnosticMode) {
    display_showDiagnostic();
    delay(200);  // обновление 5 раз/сек
    return;
  }

  static unsigned long lastPotTime = 0;
  static unsigned long lastDisplayTime = 0;
  static unsigned long lastCurrentTime = 0;

  if (modes_isReady()) {
    currMode = app_state_getMode();
    isGroupA = app_state_getGroupA();
    static Mode lastMode = app_state_getMode();
    static bool lastGroup = app_state_getGroupA();

    if (currMode != lastMode || isGroupA != lastGroup) {
      modes_reset();

      lastMode = currMode;
      lastGroup = isGroupA;
    }

    app_state_readSwitches();
    relays_setGroup(isGroupA ? GROUP_A : GROUP_B);

    if (millis() - lastPotTime >= POT_UPDATE_INTERVAL) {
      app_state_update();
      lastPotTime = millis();
    }
  }

  if (millis() - lastCurrentTime >= CURRENT_UPDATE_INTERVAL) {
    current = current_readDC();
    current_updateOverloadProtection(current);
    lastCurrentTime = millis();
  }

  if (millis() - lastDisplayTime >= DISPLAY_UPDATE_INTERVAL) {
    display_update(currMode, isGroupA, current);
    lastDisplayTime = millis();
  }

  ui_updateButtons();

  // === АВАРИЙНЫЙ РЕЖИМ: пока удерживается СТОП ===
  if (ui_isStopHeld()) {
    modes_reset();
    return;
  }

  modes_run(current, currMode, isGroupA);
}