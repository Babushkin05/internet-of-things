#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define ROWS 4
#define COLS 3

// GPIO пины
static const gpio_num_t row_pins[ROWS] = {GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14,
                                          GPIO_NUM_27};

static const gpio_num_t col_pins[COLS] = {GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33};

// Карта кнопок
static const char keymap[ROWS][COLS] = {
    {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};

static void keypad_init(void) {
  gpio_config_t io_conf = {0};

  // Настройка строк (OUTPUT)
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

  for (int i = 0; i < ROWS; i++) {
    io_conf.pin_bit_mask = 1ULL << row_pins[i];
    gpio_config(&io_conf);
    gpio_set_level(row_pins[i], 1); // по умолчанию HIGH
  }

  // Настройка колонок (INPUT + PULLUP)
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  for (int i = 0; i < COLS; i++) {
    io_conf.pin_bit_mask = 1ULL << col_pins[i];
    gpio_config(&io_conf);
  }
}

static char keypad_scan(void) {
  for (int r = 0; r < ROWS; r++) {
    // Активируем одну строку
    gpio_set_level(row_pins[r], 0);

    for (int c = 0; c < COLS; c++) {
      if (gpio_get_level(col_pins[c]) == 0) {
        esp_rom_delay_us(20000); // антидребезг ~20 мс

        if (gpio_get_level(col_pins[c]) == 0) {
          // Ждём отпускания кноп ОБЯЗАТЕЛЬНО
          while (gpio_get_level(col_pins[c]) == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
          }

          gpio_set_level(row_pins[r], 1);
          return keymap[r][c];
        }
      }
    }

    gpio_set_level(row_pins[r], 1);
  }
  return 0;
}

static void keypad_scan_debug(void) {
  for (int r = 0; r < ROWS; r++) {
    gpio_set_level(row_pins[r], 0);

    for (int c = 0; c < COLS; c++) {
      if (gpio_get_level(col_pins[c]) == 0) {
        printf("RAW: row=%d col=%d\n", r, c);
        vTaskDelay(pdMS_TO_TICKS(300));
      }
    }

    gpio_set_level(row_pins[r], 1);
  }
}

void app_main(void) {
  keypad_init();
  printf("Keypad ready\n");

  while (1) {
      char key = keypad_scan();
      if (key) {
          printf("Pressed: %c\n", key);
      }
      vTaskDelay(pdMS_TO_TICKS(50));
  }

//   while (1) {
//     keypad_scan_debug();
//     vTaskDelay(pdMS_TO_TICKS(50));
//   }
}
