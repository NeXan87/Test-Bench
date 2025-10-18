#ifndef CONFIG_H
#define CONFIG_H

// =============== ПИНЫ ===============
// Реле
#define RELAY1_PIN 4
#define RELAY2_PIN 5
#define RELAY3_PIN 2
#define RELAY4_PIN 3

// Переключатели
#define MODE0_PIN 1
#define MODE1_PIN 0
#define GROUP_PIN 6

// Кнопки
#define START1_PIN 7
#define START2_PIN 8
#define STOP_PIN 9

// Светодиоды
#define LED1_PIN 10
#define LED2_PIN 11
#define LED3_PIN 12
#define LED4_PIN 13

// Потенциометры
#define POT_CYCLES_PIN A0  // Количество цикылов
#define POT_DELAY2_PIN A1  // Задержка после реле 2/4
#define POT_ON2_PIN A2     // Время реле 2/4
#define POT_DELAY1_PIN A3  // Задержка после реле 1/3
#define POT_ON1_PIN A6     // Время реле 1/3

// Датчик тока
#define CURRENT_SENSOR_PIN A7  // Датчик постоянного тока

// =============== ДИСПЛЕЙ ===============
#define I2C_ADDR 0x27  // или 0x3F — проверьте!
#define LCD_COLS 20
#define LCD_ROWS 4

// =============== ВРЕМЕННЫЕ ДИАПАЗОНЫ (мс) ===============
#define MIN_ON_TIME 0           // 1 сек
#define MAX_ON_TIME 60000       // 60 сек
#define MIN_DELAY_TIME 0        // 0 сек (для группы A)
#define MIN_DELAY_GROUP_B 1000  // 1 сек (для группы B и режима 4)
#define MAX_DELAY_TIME 60000    // 60 сек

// =============== КАЛИБРОВКА ТОКА ===============
// ACS712-5A: 185 mV/A → 1023 / (5000/185) ≈ 37.85
// ACS712-20A: 100 mV/A → ≈ 20.46
// ACS712-30A: 66 mV/A → ≈ 13.53
#define CURRENT_SCALE 13.53f

// =============== ЦИКЛЫ ===============
#define MIN_CYCLES 1
#define MAX_CYCLES 999
#define INFINITY_THRESHOLD 1000  // analogRead() > 1000 → бесконечность

// =============== КРИВАЯ ОТОБРАЖЕНИЯ ЦИКЛОВ ===============
#define CYCLE_CURVE_POWER 1.5f  // >1 — "ускоренная" шкала (больше значений в правой части)

// =============== Границы ступеней ===============
#define STEP1_MAX        10   // до 10 → шаг 1
#define STEP2_MAX        100  // 11–100 → шаг 5
#define STEP3_MAX        500  // 101–500 → шаг 10
#define STEP4_MAX        999  // 501–999 → шаг 50

// =============== Шаги ===============
#define STEP1_SIZE       1
#define STEP2_SIZE       5
#define STEP3_SIZE       10
#define STEP4_SIZE       50


// =============== ТАЙМЕРЫ И ИНТЕРВАЛЫ ===============
#define DISPLAY_UPDATE_INTERVAL_MS 200  // Обновление дисплея
#define POT_UPDATE_INTERVAL_MS 150      // Частота опроса потенциометров
#define BLINK_INTERVAL_MS 500           // Мигание при блокировке
#define CURRENT_SAMPLES 100             // Усреднение тока
#define STARTUP_TIMEOUT 1500            // Длительность отображения заставки при включении

// =============== ОГРАНИЧЕНИЯ ===============
#define MAX_DISPLAY_TIME_SEC 6000  // 100 минут в секундах (99:59 + 1)
#define ADC_ZERO_OFFSET 512.0f     // Середина ADC (0A для ACS712)

// =============== ВРЕМЯ ЦИКЛА ===============
#define MAX_DISPLAY_TIME_HH   99
#define MAX_DISPLAY_TIME_MM   59
#define MAX_DISPLAY_TIME_SS   59

#endif