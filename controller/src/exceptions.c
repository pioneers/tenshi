// Various default exception/interrupt handlers

#include "inc/led_driver.h"

// Hard fault -- double fault or other unrecoverable error
void HardFault_Handler(void) {
  led_driver_panic();
  while (1) {}
}

// Non-maskable interrupt -- on the STM32F4xx it is only possible to trigger
// this when the crystal oscillator gets borked and stops oscillating.
void NMI_Handler(void) {
  led_driver_panic();
  while (1) {}
}
