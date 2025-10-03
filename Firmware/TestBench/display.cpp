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
  int mode,
  bool groupA,
  unsigned long cycleTimeMs,
  const char* status) {
  lcd.clear();

  // Строка 1: T1, D1, режим
  lcd.setCursor(0, 0);
  lcd.print("T1:");
  lcd.print(utils_formatTimeSec(t1));
  lcd.print(" D1:");
  lcd.print(utils_formatTimeSec(d1));
  lcd.print(" M:");
  lcd.print(mode);

  // Строка 2: T2, D2, группа
  lcd.setCursor(0, 1);
  lcd.print("T2:");
  lcd.print(utils_formatTimeSec(t2));
  lcd.print(" D2:");
  lcd.print(utils_formatTimeSec(d2));
  lcd.print(" G:");
  lcd.print(groupA ? "A" : "B");

  // Строка 3: Циклы
  lcd.setCursor(0, 2);
  lcd.print("Cycle: ");
  lcd.print(currentCycle);
  lcd.print("/");
  lcd.print(infinite ? "INF" : String(totalCycles));

  // Строка 4: Текущее время цикла, состояние работы
  lcd.setCursor(0, 3);
  lcd.print("Time: ");
  lcd.print(utils_formatCycleTime(cycleTimeMs));
  lcd.setCursor(14, 3);
  lcd.print(status);
}