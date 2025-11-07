#include "modes.h"
#include "app_state.h"
#include "relays.h"
#include "ui.h"
#include "config.h"
#include "current.h"

namespace {
enum SyncState { IDLE,
                 R1_ON,
                 D1,
                 R2_ON,
                 D2,
                 FINISHED };

// состояние машины
SyncState s_sync = IDLE;
unsigned long s_syncStart = 0;
uint16_t s_cycle = 0;
bool s_syncActive = false;

// вспомогательные состояния
bool s_relay1Locked = false;
bool s_relay2Locked = false;
bool s_anyRelayActiveInGroupB = false;

unsigned long s_cycleStartTime = 0;
bool s_cycleRunning = false;
unsigned long s_finalElapsedTime = 0;
bool g_isWorking = false;
bool g_isFinished = false;
bool g_isLocked = false;
bool g_isPaused = false;
unsigned long s_pauseStart = 0;  // время начала паузы

bool s_brakeActive = false;
bool s_brakeBlinkSlow = false;
bool s_brakeBlinkFast = false;
bool s_brakeError = false;
unsigned long s_brakeBlinkTime = 0;

struct AsyncRelay {
  bool active = false;
  bool onPhase = false;
  unsigned long phaseStart = 0;
};

AsyncRelay s_a1, s_a2;

// ---- хелперы ----
inline void setSyncState(SyncState st) {
  s_sync = st;
  s_syncStart = millis();
}

inline uint8_t currentRelay1Pin(bool groupA) {
  return groupA ? RELAY1_24V_PIN : RELAY3_380V_PIN;
}
inline uint8_t currentRelay2Pin(bool groupA) {
  return groupA ? RELAY2_24V_PIN : RELAY4_380V_PIN;
}

void resetBrakeActive() {
  s_brakeActive = false;
  s_brakeBlinkSlow = false;
  s_brakeBlinkFast = false;
  s_brakeError = false;
  s_brakeBlinkTime = 0;
}

inline void stopAllInternal() {
  relays_deactivateAll();
  ui_clearLEDs();
  resetBrakeActive();
  s_syncActive = false;
  s_sync = IDLE;
  s_cycle = 0;
  s_finalElapsedTime = 0;
  s_a1.active = s_a2.active = false;
  app_state_setCurrentCycle(0);
  current_resetOverload();
  s_cycleRunning = false;
  g_isFinished = false;
  g_isPaused = false;
  s_cycleStartTime = 0;
  s_pauseStart = 0;
}

// Обработка одного асинхронного реле (минимизация дублирования)
inline void handleAsyncRelay(AsyncRelay& ar, uint8_t pin, unsigned long tOn, unsigned long tDelay, unsigned long now) {
  if (!ar.active) return;

  if (ar.onPhase) {
    if (tOn == 0) {
      // Немедленно завершаем фазу включения
      digitalWrite(pin, LOW);
      ar.onPhase = false;
      ar.phaseStart = now;
    } else if (now - ar.phaseStart >= tOn) {
      digitalWrite(pin, LOW);
      ar.onPhase = false;
      ar.phaseStart = now;
    }
  } else {
    if (tDelay == 0) {
      // Немедленно переходим к включению
      if (tOn > 0) {
        digitalWrite(pin, HIGH);
        ar.onPhase = true;
        ar.phaseStart = now;
      } else {
        // Если и tOn = 0 — деактивируем канал полностью
        ar.active = false;
      }
    } else if (now - ar.phaseStart >= tDelay) {
      if (tOn > 0) {
        digitalWrite(pin, HIGH);
        ar.onPhase = true;
        ar.phaseStart = now;
      } else {
        ar.active = false;
      }
    }
  }
}
}

// ----------------- API -----------------

const char* modes_getStatus() {
  if (ui_isStopHeld()) return "    STOP";
  if (current_isOverload()) return "OVERLOAD";
  if (g_isFinished) return "  FINISH";
  if (g_isLocked) return "  LOCKED";
  if (g_isPaused) return "  PAUSED";
  if (s_brakeBlinkSlow) return "WT BRAKE";
  if (s_brakeBlinkFast) return "WT MOTOR";
  if (s_brakeError) return "ER BRAKE";
  if (g_isWorking) return "    WORK";
  return "   READY";
}

bool modes_isReady() {
  return !(g_isWorking || g_isPaused || g_isFinished || s_brakeBlinkSlow || s_brakeBlinkFast || s_brakeError || current_isOverload() || ui_isStopHeld());
}

bool modes_isWorking() {
  return g_isWorking;
}

bool modes_isPaused() {
  return g_isPaused;
}

bool modes_isFinished() {
  return g_isFinished;
}

void modes_init() {
  relays_deactivateAll();
}

void modes_reset() {
  stopAllInternal();
  s_relay1Locked = s_relay2Locked = false;
  s_anyRelayActiveInGroupB = false;
}

void modes_run(float current, int currMode, bool isGroupA) {
  const uint8_t r1_pin = currentRelay1Pin(isGroupA);
  const uint8_t r2_pin = currentRelay2Pin(isGroupA);

  g_isWorking = false;
  g_isFinished = false;

  // Блокировка (режимы 1 и 2 запрещены для группы B)
  const bool isBlocked = !isGroupA && currMode == MODE_ASYNC_AUTO;

  // Обновляем флаг блокировки
  g_isLocked = isBlocked;

  if (isBlocked) {
    ui_blinkAllLEDs();
    relays_deactivateAll();
    return;
  }

  // Если не заблокировано — сбрасываем флаг
  if (!isBlocked && g_isLocked) {
    g_isLocked = false;
    ui_clearLEDs();
  }

  // Режимы manual: определяем, работает ли система (есть ли реле включённые)
  if (currMode == MODE_MANUAL_BLOCKING || currMode == MODE_MANUAL_INDEPENDENT) {
    g_isFinished = false;
    bool r1On = (digitalRead(r1_pin) == HIGH);
    bool r2On = (digitalRead(r2_pin) == HIGH);
    g_isWorking = r1On || r2On;
  }

  // ---------------- MANUAL BLOCKING ----------------
  if (currMode == MODE_MANUAL_BLOCKING) {
    if (isGroupA) {
      if (ui_start1Pressed()) relays_activateFirst(true);
      if (ui_start2Pressed()) relays_activateSecond(true);
      ui_updateLEDs(true, digitalRead(r1_pin), true, digitalRead(r2_pin), 1);
    } else {
      bool r1On = (digitalRead(r1_pin) == HIGH);
      bool r2On = (digitalRead(r2_pin) == HIGH);

      if (!s_relay1Locked && ui_start1Pressed() && !r1On && !r2On) {
        digitalWrite(r1_pin, HIGH);
        s_relay1Locked = true;
        s_anyRelayActiveInGroupB = true;
      }
      if (!s_relay2Locked && ui_start2Pressed() && !r2On && !r1On) {
        digitalWrite(r2_pin, HIGH);
        s_relay2Locked = true;
        s_anyRelayActiveInGroupB = true;
      }

      digitalWrite(LED_ON1_PIN, r1On);
      digitalWrite(LED2_DELAY1_PIN, LOW);
      digitalWrite(LED_ON2_PIN, r2On);
      digitalWrite(LED4_DELAY2_PIN, LOW);
    }
    return;
  }

  // ---------------- SYNC AUTO ----------------
  if (currMode == MODE_SYNC_AUTO) {
    s_a1.active = s_a2.active = false;
    g_isFinished = (s_sync == FINISHED);
    g_isWorking = s_syncActive && !g_isFinished;

    unsigned long now = millis();

    if (current_isOverload()) {
      if (s_finalElapsedTime == 0) s_finalElapsedTime = now - s_cycleStartTime;
      return;
    }

    if (ui_start1Pressed() || ui_start2Pressed()) {
      if (!s_syncActive) {
        s_syncActive = true;
        s_cycle = 0;
        s_finalElapsedTime = 0;
        app_state_setCurrentCycle(0);
        s_cycleRunning = true;
        s_cycleStartTime = now;

        // Запуск: включаем R1 только если время > 0
        unsigned long r1Time = app_state_getRelay1Time();
        if (r1Time > 0) {
          digitalWrite(r1_pin, HIGH);
          setSyncState(R1_ON);
        } else {
          // R1 = 0 → переходим к D1
          setSyncState(D1);
        }
        return;
      }

      if (s_syncActive && !g_isPaused) {
        g_isPaused = true;
        s_pauseStart = now;
        relays_deactivateAll();
        ui_clearLEDs();
        return;
      } else if (g_isPaused) {
        g_isPaused = false;
        // Восстанавливаем состояние
        if (s_sync == R1_ON) {
          if (app_state_getRelay1Time() > 0) digitalWrite(r1_pin, HIGH);
        } else if (s_sync == R2_ON) {
          if (app_state_getRelay2Time() > 0) digitalWrite(r2_pin, HIGH);
        }
        s_cycleStartTime += (now - s_pauseStart);
        return;
      }
    }

    if (!s_syncActive) return;

    // если активна, обрабатываем состояние синхрона
    switch (s_sync) {
      case R1_ON:
        {
          if (g_isPaused) return;
          unsigned long r1Time = app_state_getRelay1Time();
          if (r1Time == 0) {
            setSyncState(D1);
          } else if ((now - s_syncStart) >= r1Time) {
            digitalWrite(r1_pin, LOW);
            setSyncState(D1);
          }
          break;
        }

      case D1:
        {
          if (g_isPaused) return;
          unsigned long d1Time = app_state_getDelay1Time();
          if (d1Time == 0) {
            // Переход к R2
            unsigned long r2Time = app_state_getRelay2Time();
            if (r2Time > 0) {
              digitalWrite(r2_pin, HIGH);
            }
            setSyncState(R2_ON);
          } else if ((now - s_syncStart) >= d1Time) {
            unsigned long r2Time = app_state_getRelay2Time();
            if (r2Time > 0) {
              digitalWrite(r2_pin, HIGH);
            }
            setSyncState(R2_ON);
          }
          break;
        }

      case R2_ON:
        {
          if (g_isPaused) return;
          unsigned long r2Time = app_state_getRelay2Time();
          if (r2Time == 0) {
            setSyncState(D2);
          } else if ((now - s_syncStart) >= r2Time) {
            digitalWrite(r2_pin, LOW);
            setSyncState(D2);
          }
          break;
        }

      case D2:
        {
          if (g_isPaused) return;
          unsigned long d2Time = app_state_getDelay2Time();
          if (d2Time == 0) {
            // Завершаем шаг цикла
            if (s_cycle <= MAX_CYCLE_COUNT) {
              s_cycle++;
              app_state_setCurrentCycle(s_cycle);
            }

            if (!app_state_getInfiniteCycles() && s_cycle >= app_state_getCycleLimit()) {
              setSyncState(FINISHED);
              s_cycleRunning = false;
              s_finalElapsedTime = now - s_cycleStartTime;
            } else {
              // Новый цикл
              unsigned long r1Time = app_state_getRelay1Time();
              if (r1Time > 0) {
                digitalWrite(r1_pin, HIGH);
                setSyncState(R1_ON);
              } else {
                setSyncState(D1);
              }
            }
          } else if ((now - s_syncStart) >= d2Time) {
            if (s_cycle <= MAX_CYCLE_COUNT) {
              s_cycle++;
              app_state_setCurrentCycle(s_cycle);
            }

            if (!app_state_getInfiniteCycles() && s_cycle >= app_state_getCycleLimit()) {
              setSyncState(FINISHED);
              s_cycleRunning = false;
              s_finalElapsedTime = now - s_cycleStartTime;
            } else {
              unsigned long r1Time = app_state_getRelay1Time();
              if (r1Time > 0) {
                digitalWrite(r1_pin, HIGH);
                setSyncState(R1_ON);
              } else {
                setSyncState(D1);
              }
            }
          }
          break;
        }

      case FINISHED:
        s_cycleRunning = false;
        digitalWrite(LED_ON1_PIN, LOW);
        digitalWrite(LED2_DELAY1_PIN, LOW);
        digitalWrite(LED_ON2_PIN, LOW);
        digitalWrite(LED4_DELAY2_PIN, LOW);
        return;

      default:
        break;
    }

    // Индикация по текущему состоянию синхрона
    digitalWrite(LED_ON1_PIN, (s_sync == R1_ON && app_state_getRelay1Time() > 0) ? HIGH : LOW);
    digitalWrite(LED2_DELAY1_PIN, (s_sync == D1 && app_state_getDelay1Time() > 0) ? HIGH : LOW);
    digitalWrite(LED_ON2_PIN, (s_sync == R2_ON && app_state_getRelay2Time() > 0) ? HIGH : LOW);
    digitalWrite(LED4_DELAY2_PIN, (s_sync == D2 && app_state_getDelay2Time() > 0) ? HIGH : LOW);
    return;
  }

  // ---------------- ASYNC AUTO ----------------
  if (currMode == MODE_ASYNC_AUTO) {
    g_isFinished = false;
    g_isWorking = s_a1.active || s_a2.active;

    unsigned long now = millis();

    if (ui_start1Pressed()) {
      s_a1.active = !s_a1.active;
      if (s_a1.active) {
        unsigned long r1Time = app_state_getRelay1Time();
        if (r1Time > 0) {
          s_a1.onPhase = true;
          s_a1.phaseStart = now;
          digitalWrite(r1_pin, HIGH);
        } else {
          // R1 = 0 → сразу переходим к задержке или выключаем
          s_a1.onPhase = false;
          s_a1.phaseStart = now;
          digitalWrite(r1_pin, LOW);
        }
      } else {
        digitalWrite(r1_pin, LOW);
      }
    }

    if (ui_start2Pressed()) {
      s_a2.active = !s_a2.active;
      if (s_a2.active) {
        unsigned long r2Time = app_state_getRelay2Time();
        if (r2Time > 0) {
          s_a2.onPhase = true;
          s_a2.phaseStart = now;
          digitalWrite(r2_pin, HIGH);
        } else {
          s_a2.onPhase = false;
          s_a2.phaseStart = now;
          digitalWrite(r2_pin, LOW);
        }
      } else {
        digitalWrite(r2_pin, LOW);
      }
    }

    handleAsyncRelay(s_a1, r1_pin, app_state_getRelay1Time(), app_state_getDelay1Time(), now);
    handleAsyncRelay(s_a2, r2_pin, app_state_getRelay2Time(), app_state_getDelay2Time(), now);

    ui_updateLEDs(
      s_a1.active, (digitalRead(r1_pin) == HIGH),
      s_a2.active, (digitalRead(r2_pin) == HIGH),
      3);
    return;
  }

  // ---------------- MANUAL INDEPENDENT ----------------
  if (currMode == MODE_MANUAL_INDEPENDENT) {
    if (isGroupA) {
      if (ui_start1Pressed()) {
        bool state = digitalRead(r1_pin);
        digitalWrite(r1_pin, !state);
      }
      if (ui_start2Pressed()) {
        bool state = digitalRead(r2_pin);
        digitalWrite(r2_pin, !state);
      }
      digitalWrite(LED_ON1_PIN, digitalRead(r1_pin));
      digitalWrite(LED2_DELAY1_PIN, LOW);
      digitalWrite(LED_ON2_PIN, digitalRead(r2_pin));
      digitalWrite(LED4_DELAY2_PIN, LOW);
      return;
    } else {
      bool r1On = (digitalRead(r1_pin) == HIGH);
      bool r2On = (digitalRead(r2_pin) == HIGH);

      // Управление тормозом
      if (!s_brakeActive) {
        if (ui_start1Pressed()) {
          s_brakeActive = true;
          s_brakeBlinkSlow = true;
          s_brakeBlinkTime = millis();
          digitalWrite(RELAY1_24V_PIN, HIGH);  // Растормозить канал 1
        } else if (ui_start2Pressed()) {
          s_brakeActive = true;
          s_brakeBlinkSlow = true;
          s_brakeBlinkTime = millis();
          digitalWrite(RELAY2_24V_PIN, HIGH);  // Растормозить канал 2
        }
      } else if (s_brakeBlinkSlow || s_brakeBlinkFast) {
        if (current >= MIN_CURRENT_BRAKE) {
          s_brakeBlinkSlow = false;
          s_brakeBlinkFast = true;

          if (!s_relay1Locked && ui_start1Pressed() && !r1On && !r2On) {
            s_relay1Locked = true;
            s_brakeBlinkSlow = false;
            s_brakeBlinkFast = false;
            digitalWrite(currentRelay1Pin(false), HIGH);
          }
          if (!s_relay2Locked && ui_start2Pressed() && !r2On && !r1On) {
            s_relay2Locked = true;
            s_brakeBlinkSlow = false;
            s_brakeBlinkFast = false;
            digitalWrite(currentRelay2Pin(false), HIGH);
          }
        } else {
          s_brakeBlinkSlow = true;
          s_brakeBlinkFast = false;
        }
      }
    }

    if (s_brakeBlinkSlow || s_brakeBlinkFast) {
      uint8_t interval = s_brakeBlinkSlow ? BRAKE_SLOW_INTERVAL : BRAKE_FAST_INTERVAL;
      if (millis() - s_brakeBlinkTime >= interval) {
        // Мигаем соответствующим светодиодом
        if (s_brakeActive && ui_start1Pressed()) {
          digitalWrite(LED_ON1_PIN, !digitalRead(LED_ON1_PIN));
        } else if (s_brakeActive && ui_start2Pressed()) {
          digitalWrite(LED_ON2_PIN, !digitalRead(LED_ON2_PIN));
        }
        s_brakeBlinkTime = millis();
      }
    } else {
      if (s_brakeBlinkTime != 0) {
        digitalWrite(LED_ON1_PIN, LOW);
        digitalWrite(LED_ON2_PIN, LOW);
        s_brakeBlinkSlow = false;
        s_brakeBlinkFast = false;
        s_brakeBlinkTime = 0;
      }

      if (s_relay1Locked) {
        digitalWrite(LED_ON1_PIN, digitalRead(r1_pin));
      }
      if (s_relay2Locked) {
        digitalWrite(LED_ON2_PIN, digitalRead(r2_pin));
      }
    }

    // Контроль тока
    if (!s_brakeError && (s_relay1Locked || s_relay2Locked)) {
      float current = current_readDC();
      if (current < MIN_CURRENT_BRAKE) {
        s_brakeError = true;
        relays_deactivateAll();
        ui_clearLEDs();
      }
    }
  }
}

unsigned long modes_getCycleElapsedTime() {
  if (s_cycleRunning && !current_isOverload()) {
    if (g_isPaused) {
      return s_pauseStart - s_cycleStartTime;
    } else {
      return millis() - s_cycleStartTime;
    }
  }
  return s_finalElapsedTime;
}