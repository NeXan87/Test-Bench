#include "modes.h"
#include "app_state.h"
#include "relays.h"
#include "ui.h"
#include "config.h"

namespace {
enum SyncState { IDLE,
                 R1_ON,
                 D1,
                 R2_ON,
                 D2,
                 FINISHED };
SyncState s_sync = IDLE;
unsigned long s_syncStart = 0;
int s_cycle = 0;
bool s_syncActive = false;
bool s_wasInBlockedMode = false;
bool s_relay1Locked = false;
bool s_relay2Locked = false;
bool s_anyRelayActiveInGroupB = false;
unsigned long s_cycleStartTime = 0;
bool s_cycleRunning = false;
bool s_waitingForUserAction = false;
unsigned long s_finalElapsedTime = 0;
bool g_isWorking = false;
bool g_isFinished = false;

struct AsyncRelay {
  bool active = false;   // включён ли автоцикл
  bool onPhase = false;  // фаза: true = включено, false = выключено
  unsigned long phaseStart = 0;
};
AsyncRelay s_a1, s_a2;

void stopAll() {
  relays_deactivateAll();
  ui_clearLEDs();
  s_syncActive = false;
  s_sync = IDLE;
  s_cycle = 0;
  s_finalElapsedTime = 0;
  s_a1.active = false;
  s_a2.active = false;
  app_state_setCurrentCycle(0);
  s_cycleRunning = false;
  s_cycleStartTime = 0;
}
}

const char* modes_getStatus() {
  if (g_isFinished) {
    return "FINISH";
  }
  return g_isWorking ? "WORK  " : "STOP  ";
}

void modes_forceIdle() {
  s_syncActive = false;
  s_sync = IDLE;
  s_waitingForUserAction = false;
  s_finalElapsedTime = 0;
}

void modes_resetCycleData() {
  s_cycle = 0;
  app_state_setCurrentCycle(0);
  s_cycleStartTime = millis();
  s_cycleRunning = false;
  s_waitingForUserAction = false;
}

bool modes_isWaitingForUserAction() {
  return s_waitingForUserAction;
}

void modes_clearWaitingState() {
  if (ui_start1Pressed() || ui_start2Pressed() || ui_stopPressed()) {
    s_waitingForUserAction = false;
  }
}

static void setSyncState(SyncState st) {
  s_sync = st;
  s_syncStart = millis();
}

void modes_init() {
  relays_deactivateAll();
}

void modes_reset() {
  stopAll();
  s_relay1Locked = false;
  s_relay2Locked = false;
  s_anyRelayActiveInGroupB = false;
}

void modes_run() {
  g_isWorking = false;
  g_isFinished = false;

  Mode mode = app_state_getMode();
  bool isCurrentGroupA = app_state_getGroupA();
  int r1 = isCurrentGroupA ? RELAY1_PIN : RELAY3_PIN;
  int r2 = isCurrentGroupA ? RELAY2_PIN : RELAY4_PIN;

  // === БЛОКИРОВКА: Режимы 3 и 4 запрещены для группы B ===
  bool isBlocked = (!isCurrentGroupA) && (mode == MODE_ASYNC_AUTO || mode == MODE_MANUAL_INDEPENDENT);

  static bool wasBlocked = false;
  if (wasBlocked && !isBlocked) {
    ui_clearLEDs();  // очистка при выходе из блокировки
  }
  wasBlocked = isBlocked;

  if (isBlocked) {
    ui_blinkAllLEDs();
    relays_deactivateAll();
    return;
  }

  // Сброс состояния "ожидания" при любом действии
  if (s_waitingForUserAction) {
    if (ui_start1Pressed() || ui_start2Pressed() || ui_stopPressed()) {
      s_cycle = 0;
      app_state_setCurrentCycle(0);
      s_cycleStartTime = millis();
      s_cycleRunning = false;
      s_waitingForUserAction = false;
    }
  }

    if (mode == MODE_MANUAL_BLOCKING || mode == MODE_MANUAL_INDEPENDENT) {
    g_isFinished = false;
    int r1 = isCurrentGroupA ? RELAY1_PIN : RELAY3_PIN;
    int r2 = isCurrentGroupA ? RELAY2_PIN : RELAY4_PIN;
    g_isWorking = (digitalRead(r1) == HIGH) || (digitalRead(r2) == HIGH);
  }

  if (mode == MODE_MANUAL_BLOCKING) {
    if (isCurrentGroupA) {
      // === Группа A: обычная взаимная блокировка ===
      if (ui_start1Pressed()) {
        relays_activateFirst(true);
      }
      if (ui_start2Pressed()) {
        relays_activateSecond(true);
      }
      ui_updateLEDs(true, digitalRead(r1), true, digitalRead(r2), 1);

    } else {
      // === Группа B: усиленная блокировка ===
      bool r1On = (digitalRead(r1) == HIGH);
      bool r2On = (digitalRead(r2) == HIGH);

      // Сброс состояния при остановке
      if (ui_stopPressed()) {
        digitalWrite(r1, LOW);
        digitalWrite(r2, LOW);
        s_relay1Locked = false;
        s_relay2Locked = false;
        s_anyRelayActiveInGroupB = false;
      } else {
        // Включение реле 1
        if (!s_relay1Locked && ui_start1Pressed() && !r1On && !r2On) {
          digitalWrite(r1, HIGH);
          s_relay1Locked = true;
          s_anyRelayActiveInGroupB = true;
        }
        // Включение реле 2
        if (!s_relay2Locked && ui_start2Pressed() && !r2On && !r1On) {
          digitalWrite(r2, HIGH);
          s_relay2Locked = true;
          s_anyRelayActiveInGroupB = true;
        }
      }

      // Индикация
      digitalWrite(LED1_PIN, r1On);
      digitalWrite(LED2_PIN, LOW);
      digitalWrite(LED3_PIN, r2On);
      digitalWrite(LED4_PIN, LOW);
    }
    return;
  }

  if (mode == MODE_SYNC_AUTO) {
    s_a1.active = false;
    s_a2.active = false;
    g_isFinished = (s_sync == FINISHED);
    g_isWorking = s_syncActive && !g_isFinished;

    if (!s_syncActive) {
      if (ui_start1Pressed() || ui_start2Pressed()) {
        s_syncActive = true;
        s_cycle = 0;
        s_finalElapsedTime = 0;
        app_state_setCurrentCycle(0);
        s_cycleRunning = true;
        s_cycleStartTime = millis();
        relays_activateFirst(true);
        setSyncState(R1_ON);
      }
    } else {
      unsigned long now = millis();
      switch (s_sync) {
        case R1_ON:
          if (now - s_syncStart >= app_state_getRelay1Time()) {
            digitalWrite(r1, LOW);
            setSyncState(D1);
          }
          break;
        case D1:
          if (now - s_syncStart >= app_state_getDelay1Time()) {
            digitalWrite(r2, HIGH);
            setSyncState(R2_ON);
          }
          break;
        case R2_ON:
          if (now - s_syncStart >= app_state_getRelay2Time()) {
            digitalWrite(r2, LOW);
            setSyncState(D2);
          }
          break;
        case D2:
          if (now - s_syncStart >= app_state_getDelay2Time()) {
            s_cycle++;
            app_state_setCurrentCycle(s_cycle);
            if (!app_state_getInfiniteCycles() && s_cycle >= app_state_getCycleLimit()) {
              setSyncState(FINISHED);
              s_cycleRunning = false;
              s_finalElapsedTime = millis() - s_cycleStartTime;
              s_waitingForUserAction = true;
              return;
            } else {
              digitalWrite(r1, HIGH);
              setSyncState(R1_ON);
            }
          }
          break;
        case FINISHED:
          s_cycleRunning = false;
          s_waitingForUserAction = true;
          digitalWrite(LED1_PIN, false);
          digitalWrite(LED2_PIN, false);
          digitalWrite(LED3_PIN, false);
          digitalWrite(LED4_PIN, false);

          // При нажатии ЛЮБОЙ кнопки — переход в IDLE
          if (ui_start1Pressed() || ui_start2Pressed() || ui_stopPressed()) {
            modes_forceIdle();
          }
          return;
      }
      digitalWrite(LED1_PIN, s_sync == R1_ON);
      digitalWrite(LED2_PIN, s_sync == D1);
      digitalWrite(LED3_PIN, s_sync == R2_ON);
      digitalWrite(LED4_PIN, s_sync == D2);
    }
    return;
  }

  if (mode == MODE_ASYNC_AUTO) {
    g_isFinished = false;
    g_isWorking = s_a1.active || s_a2.active;

    // Обработка кнопки СТОП — выключает всё
    if (ui_stopPressed()) {
      s_a1.active = false;
      s_a2.active = false;
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
    } else {
      // === Кнопка Пуск 1: переключает состояние реле 1 ===
      if (ui_start1Pressed()) {
        if (s_a1.active) {
          // Остановка реле 1
          s_a1.active = false;
          digitalWrite(r1, LOW);
        } else {
          // Запуск реле 1
          s_a1.active = true;
          s_a1.onPhase = true;
          s_a1.phaseStart = millis();
          digitalWrite(r1, HIGH);
        }
      }

      // === Кнопка Пуск 2: переключает состояние реле 2 ===
      if (ui_start2Pressed()) {
        if (s_a2.active) {
          // Остановка реле 2
          s_a2.active = false;
          digitalWrite(r2, LOW);
        } else {
          // Запуск реле 2
          s_a2.active = true;
          s_a2.onPhase = true;
          s_a2.phaseStart = millis();
          digitalWrite(r2, HIGH);
        }
      }
    }

    // === Управление циклами (только если активны) ===
    if (s_a1.active) {
      unsigned long now = millis();
      if (s_a1.onPhase) {
        if (now - s_a1.phaseStart >= app_state_getRelay1Time()) {
          digitalWrite(r1, LOW);
          s_a1.onPhase = false;
          s_a1.phaseStart = now;
        }
      } else {
        if (now - s_a1.phaseStart >= app_state_getDelay1Time()) {
          digitalWrite(r1, HIGH);
          s_a1.onPhase = true;
          s_a1.phaseStart = now;
        }
      }
    }

    if (s_a2.active) {
      unsigned long now = millis();
      if (s_a2.onPhase) {
        if (now - s_a2.phaseStart >= app_state_getRelay2Time()) {
          digitalWrite(r2, LOW);
          s_a2.onPhase = false;
          s_a2.phaseStart = now;
        }
      } else {
        if (now - s_a2.phaseStart >= app_state_getDelay2Time()) {
          digitalWrite(r2, HIGH);
          s_a2.onPhase = true;
          s_a2.phaseStart = now;
        }
      }
    }

    // Индикация
    ui_updateLEDs(
      s_a1.active, digitalRead(r1),
      s_a2.active, digitalRead(r2),
      3);
    return;
  }

  if (mode == MODE_MANUAL_INDEPENDENT) {
    // Нет сброса других реле — независимое управление
    if (ui_start1Pressed()) {
      bool r1On = digitalRead(r1);
      digitalWrite(r1, !r1On);  // переключение состояния
    }
    if (ui_start2Pressed()) {
      bool r2On = digitalRead(r2);
      digitalWrite(r2, !r2On);  // переключение состояния
    }

    // Индикация: просто отображаем текущее состояние
    digitalWrite(LED1_PIN, digitalRead(r1));
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, digitalRead(r2));
    digitalWrite(LED4_PIN, LOW);
    return;
  }
}

unsigned long modes_getCycleElapsedTime() {
  if (s_cycleRunning) {
    return millis() - s_cycleStartTime;
  }
  return s_finalElapsedTime;
}