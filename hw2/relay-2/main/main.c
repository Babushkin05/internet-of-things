// КОД БРИГАДЫ 11 (БАБУШКИН, АНОХИН, ГОБЕЦ)

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RELAY GPIO_NUM_18
void app_main(void) {
  gpio_set_direction(RELAY, GPIO_MODE_INPUT_OUTPUT);
  uint32_t ticks = 0;
  while (true) {
    printf("tick number: %ld\n", ticks);
    if (ticks % 20 == 0)
      gpio_set_level(RELAY, !gpio_get_level(RELAY));
    ticks++;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}