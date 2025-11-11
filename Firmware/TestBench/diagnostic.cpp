#include "config.h"
#include "display.h"
#include "utils.h"

void diagnostic_run() {
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
