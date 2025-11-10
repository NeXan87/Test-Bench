#include "config.h"
#include "calibration.h"
#include "display.h"
#include "utils.h"

namespace {
int16_t s_calMin[5] = { 0 };
unsigned long startTime = 0;
bool layoutDrawn = false;
bool isLeftCalibrated = false;
int prev_pOn1 = -1, prev_pD1 = -1, prev_pOn2 = -1, prev_pD2 = -1, prev_pCyc = -1;
}

int16_t readCalMin(uint8_t addr) {
  uint16_t val = EEPROM.read(addr) | (EEPROM.read(addr + 1) << 8);
  if (val == 0xFFFF) return 0;
  return (int16_t)val;
}

void writeCalMin(uint8_t addr, int16_t value) {
  EEPROM.write(addr, value & 0xFF);
  EEPROM.write(addr + 1, (value >> 8) & 0xFF);
}

int getCalibratedPercent(int raw, uint16_t minVal) {
  if (minVal > 1023) minVal = 1023;
  int corrected = raw - static_cast<int>(minVal);
  if (corrected < 0) corrected = 0;
  int range = 1023 - static_cast<int>(minVal);
  if (range <= 0) return 0;

  // Используем long для избежания переполнения
  long percent = (static_cast<long>(corrected) * 100L) / range;
  if (percent > 100) percent = 100;
  if (percent < 0) percent = 0;
  return static_cast<int>(percent);
}

// Обновление кэша из EEPROM
void calibration_load() {
  s_calMin[0] = readCalMin(ADDR_ON1_MIN);
  s_calMin[1] = readCalMin(ADDR_D1_MIN);
  s_calMin[2] = readCalMin(ADDR_ON2_MIN);
  s_calMin[3] = readCalMin(ADDR_D2_MIN);
  s_calMin[4] = readCalMin(ADDR_CYCLES_MIN);

  // Если первый запуск (все 0xFFFF), инициализируем нулями
  if (s_calMin[0] == 0xFFFF) {
    for (int i = 0; i < 5; i++) {
      s_calMin[i] = 0;
    }
  }
}

void calibration_run() {
  int pOn1 = getCalibratedPercent(analogRead(POT_ON1_PIN), s_calMin[0]);
  int pD1 = getCalibratedPercent(analogRead(POT_DELAY1_PIN), s_calMin[1]);
  int pOn2 = getCalibratedPercent(analogRead(POT_ON2_PIN), s_calMin[2]);
  int pD2 = getCalibratedPercent(analogRead(POT_DELAY2_PIN), s_calMin[3]);
  int pCyc = getCalibratedPercent(analogRead(POT_CYCLES_PIN), s_calMin[4]);

  if (!layoutDrawn) {
    lcd.setCursor(0, 0);
    lcd.print(F("R1:    D1:"));
    lcd.setCursor(0, 1);
    lcd.print(F("R2:    D2:    C:"));

    lcd.setCursor(0, 2);
    lcd.print(F("Turn all pots LEFT"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press STOP when done"));

    layoutDrawn = true;
  }

  if (isLeftCalibrated && millis() - startTime >= 1500) {
    lcd.setCursor(0, 2);
    lcd.print(F("Turn all pots RIGHT"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press STOP when done"));
    isLeftCalibrated = false;
  }

  // Строка 0: R1, D1
  if (prev_pOn1 != pOn1) {
    utils_setChars(3, 0, pOn1, true);
    prev_pOn1 = pOn1;
  }
  if (prev_pD1 != pD1) {
    utils_setChars(10, 0, pD1, true);
    prev_pD1 = pD1;
  }

  // Строка 1: R2, D2, C
  if (prev_pOn2 != pOn2) {
    utils_setChars(3, 1, pOn2, true);
    prev_pOn2 = pOn2;
  }
  if (prev_pD2 != pD2) {
    utils_setChars(10, 1, pD2, true);
    prev_pD2 = pD2;
  }
  if (prev_pCyc != pCyc) {
    utils_setChars(16, 1, pCyc, true);
    prev_pCyc = pCyc;
  }
}

void calibration_save() {
  // Сохраняем ТЕКУЩИЕ сырые значения как новые минимумы
  s_calMin[0] = analogRead(POT_ON1_PIN);
  s_calMin[1] = analogRead(POT_DELAY1_PIN);
  s_calMin[2] = analogRead(POT_ON2_PIN);
  s_calMin[3] = analogRead(POT_DELAY2_PIN);
  s_calMin[4] = analogRead(POT_CYCLES_PIN);

  writeCalMin(ADDR_ON1_MIN, s_calMin[0]);
  writeCalMin(ADDR_D1_MIN, s_calMin[1]);
  writeCalMin(ADDR_ON2_MIN, s_calMin[2]);
  writeCalMin(ADDR_D2_MIN, s_calMin[3]);
  writeCalMin(ADDR_CYCLES_MIN, s_calMin[4]);

  // Подтверждение
  lcd.setCursor(0, 3);
  lcd.print(F("Calibration SAVED"));
  isLeftCalibrated = true;
  layoutDrawn = false;
  prev_pOn1 = -1;
  prev_pD1 = -1;
  prev_pOn2 = -1;
  prev_pD2 = -1;
  prev_pCyc = -1;
}
