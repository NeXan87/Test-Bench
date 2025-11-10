#include "config.h"
#include "ui.h"
#include "relays.h"
#include "app_state.h"
#include "modes.h"
#include "display.h"
#include "current.h"
#include "calibration.h"
#include "diagnostic.h"

namespace {
bool g_isDiagnosticMode = false;
bool g_isCalibrateMode = false;
}

void setup() {
  ui_init();
  relays_init();
  app_state_init();
  modes_init();

  if (ui_isStart1Held()) {
    g_isCalibrateMode = true;
  } else if (ui_isStart2Held()) {
    g_isDiagnosticMode = true;
  }

  display_init(g_isDiagnosticMode, g_isCalibrateMode);
  ui_runStartupAnimation();
  current_setMidPoint();
  delay(STARTUP_TIMEOUT);
  calibration_load();
  ui_clearLEDs();
  display_clear();
}

void loop() {
  static unsigned long lastDisplayTime = 0;
  static unsigned long lastPotTime = 0;
  static unsigned long lastCurrentTime = 0;
  static Mode currMode = MODE_MANUAL_BLOCKING;
  static float current = 0.0f;
  static bool isGroupA = true;

  if (g_isCalibrateMode || g_isDiagnosticMode) {
    if (g_isCalibrateMode) {
      ui_updateButtons();
      if (ui_StopPressed()) calibration_save();
    }

    if (millis() - lastDisplayTime >= DISPLAY_UPDATE_INTERVAL) {
      if (g_isCalibrateMode) calibration_run();
      if (g_isDiagnosticMode) diagnostic_run();
      lastDisplayTime = millis();
    }
    return;
  }

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