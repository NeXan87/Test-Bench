#ifndef CONFIG_H
#define CONFIG_H

// =============== ПИНЫ ===============
// Реле
#define RELAY1_24V_PIN 4
#define RELAY2_24V_PIN 5
#define RELAY3_380V_PIN 2
#define RELAY4_380V_PIN 3

// Переключатели
#define MODE0_SWITCH_PIN 1
#define MODE1_SWITCH_PIN 0
#define GROUP_SWITCH_PIN 6

// Кнопки
#define START1_BUTTON_PIN 7
#define START2_BUTTON_PIN 8
#define STOP_BUTTON_PIN 9

// Светодиоды
#define LED_ON1_PIN 10
#define LED2_DELAY1_PIN 11
#define LED_ON2_PIN 12
#define LED4_DELAY2_PIN 13

// Потенциометры
#define POT_ON1_PIN A6     // Время реле 1/3
#define POT_DELAY1_PIN A3  // Задержка после реле 1/3
#define POT_ON2_PIN A2     // Время реле 2/4
#define POT_DELAY2_PIN A1  // Задержка после реле 2/4
#define POT_CYCLES_PIN A0  // Количество циклов

// Адреса минимальных и максимальных значений потенциометров в EEPROM (10 байт: uint16_t)
#define ADDR_ON1_MIN 0
#define ADDR_ON1_MAX 2
#define ADDR_D1_MIN 4
#define ADDR_D1_MAX 6
#define ADDR_ON2_MIN 8
#define ADDR_ON2_MAX 10
#define ADDR_D2_MIN 12
#define ADDR_D2_MAX 14
#define ADDR_CYCLES_MIN 16
#define ADDR_CYCLES_MAX 18

// Датчик тока
#define CURRENT_SENSOR_PIN A7  // Датчик постоянного тока

// =============== ДИСПЛЕЙ ===============
#define I2C_ADDR 0x27  // или 0x3F — проверьте!
#define LCD_COLS 20
#define LCD_ROWS 4

// =============== ВРЕМЕННЫЕ ДИАПАЗОНЫ (секунды) ===============
#define MIN_ON_RELAY1_TIME 1        // Минимальное время работы реле 1
#define MAX_ON_RELAY1_TIME 60       // Максимальное время работы реле 1
#define MIN_ON_RELAY2_TIME_SYNC 0   // Минимальное время работы реле 2 в режиме Авто (3): синхронный
#define MIN_ON_RELAY2_TIME_ASYNC 1  // Минимальное время работы реле 2 в режиме Авто (4): асинхронный
#define MAX_ON_RELAY2_TIME 60       // Максимальное время работы реле 2
#define MIN_DELAY_TIME_GROUP_A 0    // Минимальная задержка для группы A
#define MIN_DELAY_TIME_GROUP_B 1    // Минимальная задержка для группы для группы B и режима 4
#define MAX_DELAY_TIME 60           // 60 сек

// =============== КАЛИБРОВКА ТОКА ===============
// ACS712-5A: 185 mV/A
// ACS712-20A: 100 mV/A
// ACS712-30A: 66 mV/A
#define M_VPER_AMPERE 100.0f  // Чувствительность датчика ACS712. mV на 1 ампер тока. Используется для пересчёта показаний ADC в амперы.
#define CURRENT_SAMPLES 16    // Количество выборок для усреднения функции mA_DC(). Чем больше число, тем плавнее показания, но медленнее отклик.
#define CURRENT_ALPHA 0.2f    // Коэффициент сглаживания EMA фильтра (экспоненциальное скользящее среднее). 0.0 — полностью сглажено (нет реакции), 1.0 — мгновенная реакция (без фильтра)
#define VOLTAGE_ACS712 5.023f   // Напряжение питания датчика для расчёта напряжения на выходе
#define ACS_MID_POINT 515   // Показание АЦП для средней точки датчика тока (512-525)

// =============== ЗАЩИТА ПО ТОКУ ===============
#define MIN_CURRENT_BRAKE 0.3f       // Минимальный ток тормоза двигателя (А)
#define OVERLOAD_CURRENT_LIMIT 5.1f  // Порог срабатывания защиты по превышению тока (А)
#define OVERLOAD_TIME 3000           // Время удержания (мс)

// =============== ПОТЕНЦИОМЕТРЫ ===============
#define POT_ALPHA 0.3f     // Коэффициент обновления значений потенциометров. Чем меньше значение, тем быстрее
#define POT_MIN_CALIB 200  // Минимальное значение АЦП при калибровке минимального положения потенциометров
#define POT_MAX_CALIB 800  // Максимальное значение АЦП при калибровке минимального положения потенциометров

// =============== ЦИКЛЫ ===============
#define MIN_CYCLES 1
#define MAX_CYCLES 999
#define MAX_CYCLE_COUNT 9999     // Максимум посчитанных циклов (далее не будет считать)
#define INFINITY_THRESHOLD 1000  // analogRead() > 1000 → бесконечность

// =============== КРИВАЯ ОТОБРАЖЕНИЯ ЦИКЛОВ ===============
#define CYCLE_CURVE_POWER 1.5f  // > 1 — "ускоренная" шкала (больше значений в правой части)

// =============== Границы ступеней ===============
#define STEP1_MAX 10   // до 10 → шаг STEP1_SIZE
#define STEP2_MAX 100  // 11–100 → шаг STEP2_SIZE
#define STEP3_MAX 500  // 101–500 → шаг STEP3_SIZE
#define STEP4_MAX 999  // 501–999 → шаг STEP4_SIZE

// =============== Шаги ===============
#define STEP1_SIZE 1
#define STEP2_SIZE 5
#define STEP3_SIZE 10
#define STEP4_SIZE 50

// =============== ТАЙМЕРЫ И ИНТЕРВАЛЫ ===============
#define DISPLAY_UPDATE_INTERVAL 200    // Обновление дисплея
#define CURRENT_UPDATE_INTERVAL 100    // Частота опроса датчика тока
#define POT_UPDATE_INTERVAL 150        // Частота опроса потенциометров
#define BLINK_LED_INTERVAL 500         // Мигание при блокировке
#define STARTUP_TIMEOUT 1500           // Длительность отображения заставки при включении
#define BRAKE_LED_INTERVAL 500         // Частота мигания светодиода в ожидании появления тока
#define SHOW_SAVE_CALIB_TIMEOUT 1500   // Время отображения сообщения о успешном сохранении данных калибровки потенциометров
#define SHOW_ERROR_CALIB_TIMEOUT 2000  // Время отображения сообщения о ошибке позиции при калибровки потенциометров

// =============== ВРЕМЯ ЦИКЛА ===============
#define MAX_DISPLAY_TIME_HH 99
#define MAX_DISPLAY_TIME_MM 59
#define MAX_DISPLAY_TIME_SS 59

#endif