#include "app_state.h"
#include "display.h"
#include "config.h"
#include "utils.h"
#include "modes.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

void display_init() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TestBench v1.0");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  lcd.setCursor(0, 3);
  lcd.print("FW: git.new/H3iooZv");
}

void display_clear() {
  lcd.clear();
}

void display_update(uint8_t mode, bool groupA, float current) {
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
    lcd.print(F("C            I     A"));
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
  if (fabs(current - prev_current) > 0.1f) {
    lcd.setCursor(15, 2);
    if (current >= 10.0f) {
      lcd.print(current, 1);
    } else {
      lcd.print(current);
    }
    prev_current = current;
  }

  // ---- Cycle Time ----
  if (cycleTime != prev_cycleTime) {
    lcd.setCursor(2, 3);
    lcd.print(utils_formatCycleTime(cycleTime));
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