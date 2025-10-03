#include "config.h"
#include "ui.h"
#include "relays.h"
#include "app_state.h"
#include "modes.h"
#include "display.h"

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
  app_state_update();

  // Сброс по кнопкам
  if (modes_isWaitingForUserAction()) {
    if (ui_start1Pressed() || ui_start2Pressed() || ui_stopPressed()) {
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

  if (ui_stopPressed()) {
    relays_deactivateAll();
    modes_reset();
  }

  if (modes_isWaitingForUserAction()) {
    if (ui_start1Pressed() || ui_start2Pressed() || ui_stopPressed()) {
      modes_clearWaitingState();
    }
  }

  modes_run();

  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay >= DISPLAY_UPDATE_INTERVAL_MS) {
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
      modes_getStatus());
    lastDisplay = millis();
  }

  delay(10);
}