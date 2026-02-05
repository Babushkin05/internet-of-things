// КОД БРИГАДЫ 11 (БАБУШКИН, АНОХИН, ГОБЕЦ)

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#define ROWS 4
#define COLS 3

#define RELAY_GPIO GPIO_NUM_18
#define LED_GPIO   GPIO_NUM_2

#define CODE_LENGTH 4
static const char CORRECT_CODE[CODE_LENGTH + 1] = "1337";

// ---------- КЛАВИАТУРА ----------

static const gpio_num_t row_pins[ROWS] = {
    GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14, GPIO_NUM_27};

static const gpio_num_t col_pins[COLS] = {
    GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33};

static const char keymap[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

static void keypad_init(void) {
    gpio_config_t io_conf = {0};

    io_conf.mode = GPIO_MODE_OUTPUT;
    for (int i = 0; i < ROWS; i++) {
        io_conf.pin_bit_mask = 1ULL << row_pins[i];
        gpio_config(&io_conf);
        gpio_set_level(row_pins[i], 1);
    }

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    for (int i = 0; i < COLS; i++) {
        io_conf.pin_bit_mask = 1ULL << col_pins[i];
        gpio_config(&io_conf);
    }
}

static char keypad_scan(void) {
    for (int r = 0; r < ROWS; r++) {
        gpio_set_level(row_pins[r], 0);

        for (int c = 0; c < COLS; c++) {
            if (gpio_get_level(col_pins[c]) == 0) {
                esp_rom_delay_us(20000);

                if (gpio_get_level(col_pins[c]) == 0) {
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

// ---------- ЗАМОК ----------

static void lock_init(void) {
    gpio_config_t io_conf = {0};

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << RELAY_GPIO) | (1ULL << LED_GPIO);
    gpio_config(&io_conf);

    gpio_set_level(RELAY_GPIO, 0); // дверь закрыта
    gpio_set_level(LED_GPIO, 1);   // светодиод горит
}

static void open_door(void) {
    printf("Access granted\n");

    gpio_set_level(RELAY_GPIO, 1); // открыть
    for (int i = 0; i < 20; i++) { // 10 секунд, мигание
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    gpio_set_level(RELAY_GPIO, 0); // закрыть
    gpio_set_level(LED_GPIO, 1);   // постоянный свет
}

// ---------- APP_MAIN ----------

void app_main(void) {
    keypad_init();
    lock_init();

    char entered_code[CODE_LENGTH + 1] = {0};
    int index = 0;

    printf("System ready\n");

    while (1) {
        char key = keypad_scan();
        if (key) {
            if (index < CODE_LENGTH) {
                entered_code[index++] = key;

                // печать маски
                for (int i = 0; i < CODE_LENGTH; i++) {
                    if (i < index)
                        printf("%c", entered_code[i]);
                    else
                        printf("*");
                }
                printf("\n");
            }

            if (index == CODE_LENGTH) {
                if (strcmp(entered_code, CORRECT_CODE) == 0) {
                    open_door();
                } else {
                    printf("wrong code\n");
                }

                memset(entered_code, 0, sizeof(entered_code));
                index = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
