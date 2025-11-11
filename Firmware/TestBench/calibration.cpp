#include "config.h"
#include "calibration.h"
#include "display.h"
#include "utils.h"
#include <EEPROM.h>

namespace {
int16_t s_calMin[5] = { 0 };
int16_t s_calMax[5] = { 1023 };
unsigned long s_savedTime = 0;
unsigned long s_errorTime = 0;
enum CalState { CAL_LEFT,
                CAL_RIGHT,
                CAL_SAVE,
                CAL_DONE,
                CAL_ERROR };
enum ErrorType { CAL_ERROR_LEFT,
                 CAL_ERROR_RIGHT };
CalState s_state = CAL_LEFT;
ErrorType s_errorType = CAL_ERROR_LEFT;
int prev_pOn1 = -1, prev_pD1 = -1, prev_pOn2 = -1, prev_pD2 = -1, prev_pCyc = -1;
bool s_layoutDrawn = false;
bool s_instructionsDrawn = false;
bool s_errorDrawn = false;
}

static int16_t readCalValue(uint8_t addr) {
  uint16_t val = EEPROM.read(addr) | (EEPROM.read(addr + 1) << 8);
  if (val == 0xFFFF) return (addr % 2 == 0) ? 0 : 1023;
  return (int16_t)val;
}

static void writeCalValue(uint8_t addr, int16_t value) {
  EEPROM.write(addr, value & 0xFF);
  EEPROM.write(addr + 1, (value >> 8) & 0xFF);
}

static int getCalibratedValue(int raw, int16_t minVal, int16_t maxVal, bool returnPercent = true) {
  if (maxVal <= minVal) {
    // Защита: если калибровка некорректна, возвращаем 0 или raw
    return returnPercent ? 0 : raw;
  }

  long range = (long)maxVal - (long)minVal;
  long corrected = (long)raw - (long)minVal;
  if (corrected < 0) corrected = 0;
  if (corrected > range) corrected = range;

  if (returnPercent) {
    long percent = (corrected * 100L) / range;
    if (percent > 100) percent = 100;
    if (percent < 0) percent = 0;
    return (int)percent;
  } else {
    // Возвращаем откалиброванное значение в диапазоне 0–1023
    long scaled = (corrected * 1023L) / range;
    if (scaled > 1023) scaled = 1023;
    if (scaled < 0) scaled = 0;
    return (int)scaled;
  }
}

void calibration_load() {
  s_calMin[0] = readCalValue(ADDR_ON1_MIN);
  s_calMin[1] = readCalValue(ADDR_D1_MIN);
  s_calMin[2] = readCalValue(ADDR_ON2_MIN);
  s_calMin[3] = readCalValue(ADDR_D2_MIN);
  s_calMin[4] = readCalValue(ADDR_CYCLES_MIN);

  s_calMax[0] = readCalValue(ADDR_ON1_MAX);
  s_calMax[1] = readCalValue(ADDR_D1_MAX);
  s_calMax[2] = readCalValue(ADDR_ON2_MAX);
  s_calMax[3] = readCalValue(ADDR_D2_MAX);
  s_calMax[4] = readCalValue(ADDR_CYCLES_MAX);

  // Проверяем, были ли все значения сохранены (не 0xFFFF)
  bool allValid = true;
  for (int i = 0; i < 5; i++) {
    if (s_calMin[i] == 0xFFFF || s_calMax[i] == 0xFFFF) {
      allValid = false;
      break;
    }
  }

  if (!allValid) {
    // Если есть мусор — сбрасываем всё в 0 и 1023
    for (int i = 0; i < 5; i++) {
      s_calMin[i] = 0;
      s_calMax[i] = 1023;
    }
  } else {
    // Защита от некорректных значений
    for (int i = 0; i < 5; i++) {
      if (s_calMin[i] > 1023) s_calMin[i] = 0;
      if (s_calMax[i] > 1023) s_calMax[i] = 1023;
      if (s_calMin[i] >= s_calMax[i]) {
        s_calMin[i] = 0;
        s_calMax[i] = 1023;
      }
    }
  }
}

bool calibration_isSaving() {
  return (s_state == CAL_LEFT) && (millis() - s_savedTime < SHOW_SAVE_CALIB_TIMEOUT);
}

bool calibration_isInError() {
  return (s_state == CAL_ERROR) && (millis() - s_errorTime < SHOW_ERROR_CALIB_TIMEOUT);
}

void calibration_run() {
  int raw[5] = {
    analogRead(POT_ON1_PIN),
    analogRead(POT_DELAY1_PIN),
    analogRead(POT_ON2_PIN),
    analogRead(POT_DELAY2_PIN),
    analogRead(POT_CYCLES_PIN)
  };

  int pOn1 = getCalibratedValue(raw[0], s_calMin[0], s_calMax[0]);
  int pD1 = getCalibratedValue(raw[1], s_calMin[1], s_calMax[1]);
  int pOn2 = getCalibratedValue(raw[2], s_calMin[2], s_calMax[2]);
  int pD2 = getCalibratedValue(raw[3], s_calMin[3], s_calMax[3]);
  int pCyc = getCalibratedValue(raw[4], s_calMin[4], s_calMax[4]);

  // Статичный layout — один раз
  if (!s_layoutDrawn) {
    lcd.setCursor(0, 0);
    lcd.print(F("R1:"));
    lcd.setCursor(7, 0);
    lcd.print(F("D1:"));
    lcd.setCursor(0, 1);
    lcd.print(F("R2:"));
    lcd.setCursor(7, 1);
    lcd.print(F("D2:"));
    lcd.setCursor(14, 1);
    lcd.print(F("C:"));
    s_layoutDrawn = true;
  }

  if (prev_pOn1 != pOn1) {
    utils_setChars(3, 0, pOn1, true);
    prev_pOn1 = pOn1;
  }
  if (prev_pD1 != pD1) {
    utils_setChars(10, 0, pD1, true);
    prev_pD1 = pD1;
  }
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

  // Обработка ошибки
  if (calibration_isInError()) {
    if (!s_errorDrawn) {
      lcd.setCursor(0, 2);
      lcd.print(F("Pots not in position   "));
      lcd.setCursor(0, 3);
      lcd.print(F("                       "));
      s_layoutDrawn = false;
      s_errorDrawn = true;
    }
    return;
  } else if (s_state == CAL_ERROR) {
    // Выход из ошибки → возврат к предыдущему состоянию
    s_errorDrawn = false; 
    s_state = (s_errorType == CAL_ERROR_LEFT) ? CAL_LEFT : CAL_RIGHT;
    s_instructionsDrawn = false;
  }

  // Обновление инструкций
  if (!s_instructionsDrawn) {
    if (s_state == CAL_LEFT) {
      lcd.setCursor(0, 2);
      lcd.print(F("Turn all pots LEFT   "));
      lcd.setCursor(0, 3);
      lcd.print(F("Press STOP when done "));
      s_layoutDrawn = false;
    } else if (s_state == CAL_RIGHT) {
      lcd.setCursor(0, 2);
      lcd.print(F("Turn all pots RIGHT  "));
      lcd.setCursor(0, 3);
      lcd.print(F("Press STOP when done "));
      s_layoutDrawn = false;
    } else if (s_state == CAL_DONE) {
      lcd.setCursor(0, 2);
      lcd.print(F("Calibration DONE    "));
      lcd.setCursor(0, 3);
      lcd.print(F("Restart the bench   "));
      s_layoutDrawn = false;
    }
    s_instructionsDrawn = true;
  }

  // Автоматический переход после SAVED
  if (s_state == CAL_SAVE && millis() - s_savedTime >= SHOW_SAVE_CALIB_TIMEOUT) {
    s_state = CAL_RIGHT;
    s_instructionsDrawn = false;
  }
}

void calibration_save() {
  if (calibration_isSaving() || s_state == CAL_DONE || s_state == CAL_ERROR) {
    return;
  }

  // Считываем текущие значения
  int raw[5] = {
    analogRead(POT_ON1_PIN),
    analogRead(POT_DELAY1_PIN),
    analogRead(POT_ON2_PIN),
    analogRead(POT_DELAY2_PIN),
    analogRead(POT_CYCLES_PIN)
  };

  bool valid = true;

  if (s_state == CAL_LEFT) {
    for (int i = 0; i < 5; i++) {
      if (raw[i] >= POT_MIN_CALIB) {
        valid = false;
        break;
      }
    }
    if (!valid) {
      s_state = CAL_ERROR;
      s_errorType = CAL_ERROR_LEFT;
      s_errorTime = millis();
      s_instructionsDrawn = false;
      return;
    }

    // Сохраняем MIN
    s_calMin[0] = raw[0];
    writeCalValue(ADDR_ON1_MIN, raw[0]);
    s_calMin[1] = raw[1];
    writeCalValue(ADDR_D1_MIN, raw[1]);
    s_calMin[2] = raw[2];
    writeCalValue(ADDR_ON2_MIN, raw[2]);
    s_calMin[3] = raw[3];
    writeCalValue(ADDR_D2_MIN, raw[3]);
    s_calMin[4] = raw[4];
    writeCalValue(ADDR_CYCLES_MIN, raw[4]);

    lcd.setCursor(0, 2);
    lcd.print(F("Calibration SAVED!  "));
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));
    s_savedTime = millis();
    s_instructionsDrawn = false;
    s_state = CAL_SAVE;

  } else if (s_state == CAL_RIGHT) {
    for (int i = 0; i < 5; i++) {
      if (raw[i] <= POT_MAX_CALIB) {
        valid = false;
        break;
      }
    }
    if (!valid) {
      s_state = CAL_ERROR;
      s_errorType = CAL_ERROR_RIGHT;
      s_errorTime = millis();
      s_instructionsDrawn = false;
      return;
    }

    // Сохраняем MAX
    s_calMax[0] = raw[0];
    writeCalValue(ADDR_ON1_MAX, raw[0]);
    s_calMax[1] = raw[1];
    writeCalValue(ADDR_D1_MAX, raw[1]);
    s_calMax[2] = raw[2];
    writeCalValue(ADDR_ON2_MAX, raw[2]);
    s_calMax[3] = raw[3];
    writeCalValue(ADDR_D2_MAX, raw[3]);
    s_calMax[4] = raw[4];
    writeCalValue(ADDR_CYCLES_MAX, raw[4]);

    s_state = CAL_DONE;
    s_instructionsDrawn = false;
  }
}

int calibration_getCalibratedADC(int raw, uint8_t potIndex) {
  if (potIndex >= 5) return 0;
  return getCalibratedValue(raw, s_calMin[potIndex], s_calMax[potIndex], false);
}