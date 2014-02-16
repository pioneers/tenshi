#include <stdio.h>

#include "inc/FreeRTOS.h"
#include "inc/led_driver.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/task.h"

static portTASK_FUNCTION_PROTO(blinkTask, pvParameters) {
  while (1) {
    // Blink green LED
    // GPIO_BANK(PINDEF_GREEN_LED)->ODR ^=
    //   (1 << GPIO_PIN(PINDEF_GREEN_LED));

    // debug_uart_tx("Hello World!\r\n", 14);

    vTaskDelay(1000);
  }
}

int main(int argc, char **argv) {
  led_driver_init();
  // Enable GPIOD clock
  // RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  // Enable GPIOB clock
  // RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  // TODO(rqou): Need some kind of io_init function

  // Set red and green LED GPIO output
  // CONFIGURE_IO(RED_LED);
  // CONFIGURE_IO(GREEN_LED);
  // Turn on red LED
  // GPIO_BANK(PINDEF_RED_LED)->ODR =
  //   (1 << GPIO_PIN(PINDEF_RED_LED));

  // debug_uart_setup();
  
  xTaskCreate(blinkTask, "Blink", 256, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}
