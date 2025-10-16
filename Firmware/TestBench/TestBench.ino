#include "config.h"
#include "ui.h"
#include "relays.h"
#include "app_state.h"
#include "modes.h"
#include "display.h"
#include "current.h"

void setup() {
  ui_init();
  relays_init();
  app_state_init();
  modes_init();
  ui_runStartupAnimation();
  display_init();
  delay(STARTUP_TIMEOUT);
  ui_clearLEDs();
  display_clear();
}

void loop() {
  ui_updateButtons();

  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay >= DISPLAY_UPDATE_INTERVAL_MS) {
    float current = 0;

    display_update(
      app_state_getRelay1Time(),
      app_state_getDelay1Time(),
      app_state_getRelay2Time(),
      app_state_getDelay2Time(),
      app_state_getCurrentCycle(),
      app_state_getCycleLimit(),
      app_state_getInfiniteCycles(),
      (int)app_state_getMode(),
      app_state_getGroupA(),
      modes_getCycleElapsedTime(),
      modes_getStatus(),
      current);
    lastDisplay = millis();
  }

  // === АВАРИЙНЫЙ РЕЖИМ: пока удерживается СТОП ===
  if (ui_isStopHeld()) {
    modes_reset();
    delay(10);
    return;
  }

  static unsigned long lastPotUpdate = 0;
  if (!modes_isWorking() && millis() - lastPotUpdate >= POT_UPDATE_INTERVAL_MS) {
    app_state_update();
    lastPotUpdate = millis();
  }

  // Сброс после аварии
  // if (ui_stopReleased()) {

  // }

  // Сброс по кнопкам
  if (modes_isWaitingForUserAction()) {
    if (ui_start1Pressed() || ui_start2Pressed()) {
      modes_resetCycleData();
    }
  }

  // Сброс по переключателям
  static Mode lastMode = MODE_MANUAL_BLOCKING;
  static bool lastGroup = true;

  if (app_state_isIdle()) {
    Mode currMode = app_state_getMode();
    bool currGroup = app_state_getGroupA();

    if (currMode != lastMode || currGroup != lastGroup) {
      if (modes_isWaitingForUserAction()) {
        modes_resetCycleData();
      }
      modes_forceIdle();
      lastMode = currMode;
      lastGroup = currGroup;
    }

    app_state_readSwitches();
    relays_setGroup(currGroup ? GROUP_A : GROUP_B);
  }

  if (modes_isWaitingForUserAction()) {
    if (ui_start1Pressed() || ui_start2Pressed()) {
      modes_clearWaitingState();
    }
  }

  modes_run();
}