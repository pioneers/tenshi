#include <stdio.h>

#include "inc/FreeRTOS.h"
#include "inc/debug_uart.h"
#include "inc/stm32f4xx.h"
#include "inc/task.h"

static portTASK_FUNCTION_PROTO(blinkTask, pvParameters) {
  while (1) {
    // Blink green LED
    GPIOD->ODR ^= (1 << 12);

    debug_uart_tx("Hello World!\r\n", 14);

    vTaskDelay(1000);
  }
}

int main(int argc, char **argv) {
  // Enable GPIOD clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  // Enable GPIOB clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  // TODO(rqou): Need some kind of io_init function

  // Set red and green LED GPIO output
  GPIOD->MODER |= (1 << (12 * 2)) | (1 << (14 * 2));
  // Turn on red LED
  GPIOD->ODR = (1 << 14);

  debug_uart_setup();

  xTaskCreate(blinkTask, "Blink", 256, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}
