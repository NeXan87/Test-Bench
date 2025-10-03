#ifndef CONFIG_H
#define CONFIG_H

// Реле
#define RELAY1_PIN 1
#define RELAY2_PIN 0
#define RELAY3_PIN 2
#define RELAY4_PIN 3

// Переключатели
#define MODE0_PIN 4
#define MODE1_PIN 5
#define GROUP_PIN 6

// Кнопки
#define START1_PIN 7
#define START2_PIN 8
#define STOP_PIN   9

// Светодиоды
#define LED1_PIN 10
#define LED2_PIN 11
#define LED3_PIN 12
#define LED4_PIN 13

// Потенциометры
#define POT_CYCLES_PIN   A0  // Количество циклов
#define POT_DELAY2_PIN   A1  // Задержка после реле 2/4
#define POT_ON2_PIN      A2  // Время реле 2/4
#define POT_DELAY1_PIN   A3  // Задержка после реле 1/3
#define POT_ON1_PIN      A6  // Время реле 1/3 ← перенесено на A6!

// I2C дисплей
#define I2C_ADDR 0x20  // или 0x3F — проверьте!
#define LCD_COLS 20
#define LCD_ROWS 4

#endif