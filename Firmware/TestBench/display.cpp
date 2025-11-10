
#include "display.h"
#include "config.h"
#include "utils.h"
#include "ui.h"
#include "modes.h"

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

void display_init(bool isDiagnosticMode, bool g_isCalibrateMode) {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("TestBench v1.0"));
  lcd.setCursor(0, 1);
  if (isDiagnosticMode) {
    lcd.print(F("DIAGNOSTIC MODE"));
  } else if (g_isCalibrateMode) {
    lcd.print(F("CALIBRATE MODE"));
  } else {
    lcd.print(F("Starting..."));
  }
  lcd.setCursor(0, 3);
  lcd.print(F("FW: git.new/H3iooZv"));
}

void display_clear() {
  lcd.clear();
}

void display_showDiagnostic() {
  static bool layoutDrawn = false;
  static int prev_d0 = -1, prev_d1 = -1, prev_d6 = -1, prev_d7 = -1, prev_d8 = -1, prev_d9 = -1;
  static int prev_a0 = -1, prev_a1 = -1, prev_a2 = -1, prev_a3 = -1, prev_a6 = -1, prev_a7 = -1;

  if (!layoutDrawn) {
    lcd.setCursor(0, 0);
    lcd.print(F("D0=  D8=     A2="));
    lcd.setCursor(0, 1);
    lcd.print(F("D1=  D9=     A3="));
    lcd.setCursor(0, 2);
    lcd.print(F("D6=  A0=     A6="));
    lcd.setCursor(0, 3);
    lcd.print(F("D7=  A1=     A7="));

    layoutDrawn = true;
  }

  // Считываем состояния
  uint8_t d0 = digitalRead(0);
  uint8_t d1 = digitalRead(1);
  uint8_t d6 = digitalRead(6);
  uint8_t d7 = digitalRead(7);
  uint8_t d8 = digitalRead(8);
  uint8_t d9 = digitalRead(9);

  int a0 = analogRead(A0);
  int a1 = analogRead(A1);
  int a2 = analogRead(A2);
  int a3 = analogRead(A3);
  int a6 = analogRead(A6);
  int a7 = analogRead(A7);

  // Строка 0: D0, D8, A2
  if (prev_d0 != d0) {
    utils_setChars(3, 0, d0);
    prev_d0 = d0;
  }
  if (prev_d8 != d8) {
    utils_setChars(8, 0, d8);
    prev_d8 = d8;
  }
  if (prev_a2 != a2) {
    utils_setChars(16, 0, a2, true);
    prev_a2 = a2;
  }

  // Строка 1: D1, D9, A3
  if (prev_d1 != d1) {
    utils_setChars(3, 1, d1);
    prev_d1 = d1;
  }
  if (prev_d9 != d9) {
    utils_setChars(8, 1, d9);
    prev_d9 = d9;
  }
  if (prev_a3 != a3) {
    utils_setChars(16, 1, a3, true);
    prev_a3 = a3;
  }

  // Строка 2: D6, A0, A6
  if (prev_d6 != d6) {
    utils_setChars(3, 2, d6);
    prev_d6 = d6;
  }
  if (prev_a0 != a0) {
    utils_setChars(8, 2, a0, true);
    prev_a0 = a0;
  }
  if (prev_a6 != a6) {
    utils_setChars(16, 2, a6, true);
    prev_a6 = a6;
  }

  // Строка 3: D7, A1, A7
  if (prev_d7 != d7) {
    utils_setChars(3, 3, d7);
    prev_d7 = d7;
  }
  if (prev_a1 != a1) {
    utils_setChars(8, 3, a1, true);
    prev_a1 = a1;
  }
  if (prev_a7 != a7) {
    utils_setChars(16, 3, a7, true);
    prev_a7 = a7;
  }
}

void display_update(Mode mode, bool groupA, float current) {
  // --- статические значения для сравнения ---
  static bool layoutDrawn = false;  // ← чтобы макет выводился один раз
  static unsigned long prev_relay1Time = 0, prev_delay1Time = 0, prev_relay2Time = 0, prev_delay2Time = 0, prev_cycleTime = -1;
  static uint16_t prev_currentCycle = 0, prev_totalCycles = 0;
  static bool prev_isInfiniteCycles = false, prev_groupA = false;
  static uint8_t prev_mode = 255;
  static float prev_current = -1.0;
  static char prev_status[17] = "";
  char buffer[12];

  unsigned long relay1Time = app_state_getRelay1Time();
  unsigned long delay1Time = app_state_getDelay1Time();
  unsigned long relay2Time = app_state_getRelay2Time();
  unsigned long delay2Time = app_state_getDelay2Time();
  unsigned long cycleTime = modes_getCycleElapsedTime();
  int currentCycle = app_state_getCurrentCycle();
  int totalCycles = app_state_getCycleLimit();
  bool isInfiniteCycles = app_state_getInfiniteCycles();
  const char* status = modes_getStatus();

  // ===========================================================
  // 1️⃣ Разметка экрана (печатается один раз при первом вызове)
  // ===========================================================
  if (!layoutDrawn) {
    static const uint8_t ARROW_POSITIONS[] = { 2, 10, 17, 2, 10, 17, 1, 14, 1 };  // позиции
    static const uint8_t ARROW_ROWS[] = { 0, 0, 0, 1, 1, 1, 2, 2, 3 };            // строки
    static const uint8_t ARROW_COUNT = sizeof(ARROW_POSITIONS);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("R1   s  D1   s  M"));
    lcd.setCursor(0, 1);
    lcd.print(F("R2   s  D2   s  G"));
    lcd.setCursor(0, 2);
    lcd.print(F("C            I"));
    lcd.setCursor(0, 3);
    lcd.print(F("T"));

    for (uint8_t i = 0; i < ARROW_COUNT; i++) {
      lcd.setCursor(ARROW_POSITIONS[i], ARROW_ROWS[i]);
      lcd.print(char(126));  // символ '→'
    }

    layoutDrawn = true;
  }

  // ===========================================================
  // 2️⃣ Умное обновление значений
  // ===========================================================

  // ---- relay1Time / delay1Time / Mode ----
  if (relay1Time != prev_relay1Time) {
    lcd.setCursor(3, 0);
    lcd.print(utils_formatTimeSec(relay1Time));
    prev_relay1Time = relay1Time;
  }

  if (delay1Time != prev_delay1Time) {
    lcd.setCursor(11, 0);
    lcd.print(utils_formatTimeSec(delay1Time));
    prev_delay1Time = delay1Time;
  }

  if (mode != prev_mode) {
    lcd.setCursor(18, 0);
    lcd.print(mode);
    prev_mode = mode;
  }

  // ---- relay2Time / delay2Time / Group ----
  if (relay2Time != prev_relay2Time) {
    lcd.setCursor(3, 1);
    lcd.print(utils_formatTimeSec(relay2Time));
    prev_relay2Time = relay2Time;
  }

  if (delay2Time != prev_delay2Time) {
    lcd.setCursor(11, 1);
    lcd.print(utils_formatTimeSec(delay2Time));
    prev_delay2Time = delay2Time;
  }

  if (groupA != prev_groupA) {
    lcd.setCursor(18, 1);
    lcd.print(groupA ? F("A") : F("B"));
    prev_groupA = groupA;
  }

  // ---- Cycle counter ----
  if (currentCycle != prev_currentCycle || totalCycles != prev_totalCycles || isInfiniteCycles != prev_isInfiniteCycles) {
    lcd.setCursor(2, 2);
    if (isInfiniteCycles)
      snprintf(buffer, sizeof(buffer), "%04d/INF ", currentCycle);
    else
      snprintf(buffer, sizeof(buffer), "%04d/%04d", currentCycle, totalCycles);
    lcd.print(buffer);
    prev_currentCycle = currentCycle;
    prev_totalCycles = totalCycles;
    prev_isInfiniteCycles = isInfiniteCycles;
  }

  // ---- Current ----
  if (fabs(current - prev_current) > 0.01f) {
    lcd.setCursor(15, 2);
    lcd.print(current, 1);
    lcd.print(F("A "));
    prev_current = current;
  }

  // ---- Cycle Time ----
  if (cycleTime != prev_cycleTime) {
    lcd.setCursor(2, 3);
    char cycleTimeStr[9];
    utils_formatCycleTime(cycleTime, cycleTimeStr, sizeof(cycleTimeStr));
    lcd.print(cycleTimeStr);
    prev_cycleTime = cycleTime;
  }

  // ---- Status ----
  if (strncmp(status, prev_status, sizeof(prev_status)) != 0) {
    lcd.setCursor(12, 3);
    lcd.print(status);
    strncpy(prev_status, status, sizeof(prev_status) - 1);
    prev_status[sizeof(prev_status) - 1] = '\0';
  }
}