#include "display.h"
#include "config.h"
#include "utils.h"
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

void display_update(
  unsigned long t1, unsigned long d1,
  unsigned long t2, unsigned long d2,
  int currentCycle,
  int totalCycles,
  bool infinite,
  uint8_t mode,
  bool groupA,
  unsigned long cycleTimeMs,
  const char* status,
  float current) {

  // --- статические значения для сравнения ---
  static bool layoutDrawn = false;  // ← чтобы макет выводился один раз
  static unsigned long prev_t1 = 0, prev_d1 = 0, prev_t2 = 0, prev_d2 = 0, prev_cycleTime = -1;
  static uint16_t prev_currentCycle = 0, prev_totalCycles = 0;
  static bool prev_infinite = false, prev_groupA = false;
  static uint8_t prev_mode = 255;
  static float prev_current = -1.0;
  static char prev_status[17] = "";

  char buffer[12];

  // ===========================================================
  // 1️⃣ Разметка экрана (печатается один раз при первом вызове)
  // ===========================================================
  if (!layoutDrawn) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(F("R1:  s D1:  s M:"));
    lcd.setCursor(0, 1); lcd.print(F("R2:  s D2:  s G:"));
    lcd.setCursor(0, 2); lcd.print(F("Cycl:         I: . A"));
    lcd.setCursor(0, 3); lcd.print(F("Time:"));
    layoutDrawn = true;
  }

  // ===========================================================
  // 2️⃣ Умное обновление значений
  // ===========================================================

  // ---- T1 / D1 / Mode ----
  if (t1 != prev_t1) {
    lcd.setCursor(3, 0);
    lcd.print(utils_formatTimeSec(t1));
    prev_t1 = t1;
  }

  if (d1 != prev_d1) {
    lcd.setCursor(10, 0);
    lcd.print(utils_formatTimeSec(d1));
    prev_d1 = d1;
  }

  if (mode != prev_mode) {
    lcd.setCursor(16, 0);
    lcd.print(mode);
    prev_mode = mode;
  }

  // ---- T2 / D2 / Group ----
  if (t2 != prev_t2) {
    lcd.setCursor(3, 1);
    lcd.print(utils_formatTimeSec(t2));
    prev_t2 = t2;
  }

  if (d2 != prev_d2) {
    lcd.setCursor(10, 1);
    lcd.print(utils_formatTimeSec(d2));
    prev_d2 = d2;
  }

  if (groupA != prev_groupA) {
    lcd.setCursor(16, 1);
    lcd.print(groupA ? F("A") : F("B"));
    prev_groupA = groupA;
  }

  // ---- Cycle counter ----
  if (currentCycle != prev_currentCycle || totalCycles != prev_totalCycles || infinite != prev_infinite) {
    lcd.setCursor(5, 2);
    if (infinite)
      snprintf(buffer, sizeof(buffer), "%03d/INF", currentCycle);
    else
      snprintf(buffer, sizeof(buffer), "%03d/%03d", currentCycle, totalCycles);
    lcd.print(buffer);
    prev_currentCycle = currentCycle;
    prev_totalCycles = totalCycles;
    prev_infinite = infinite;
  }

  // ---- Current ----
  if (fabs(current - prev_current) > 0.05f) {
    lcd.setCursor(16, 2);
    uint8_t whole = (uint8_t)current;
    uint8_t decimal = (uint8_t)((current - whole) * 10 + 0.5f);
    lcd.print(whole);
    lcd.setCursor(18, 2);
    lcd.print(decimal);
    prev_current = current;
  }

  // ---- Cycle Time ----
  if (cycleTimeMs != prev_cycleTime) {
    lcd.setCursor(5, 3);
    lcd.print(utils_formatCycleTime(cycleTimeMs));
    prev_cycleTime = cycleTimeMs;
  }

  // ---- Status ----
  if (strncmp(status, prev_status, sizeof(prev_status)) != 0) {
    lcd.setCursor(14, 3);
    lcd.print(status);
    strncpy(prev_status, status, sizeof(prev_status) - 1);
    prev_status[sizeof(prev_status) - 1] = '\0';
  }
}