#include "inc/button_driver.h"

#include "inc/FreeRTOS.h"
#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/pindef.h"
#include "inc/task.h"

static volatile int button0_state = 0;
static volatile int button1_state = 0;

#define DEBOUNCE_DELAY  (50 / portTICK_RATE_MS)

void button_driver_init(void) {
  // Enable SYSCFG clock so that EXTI can be set
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_BUTTON0);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_BUTTON1);

  // Configure the ports
  CONFIGURE_IO(BUTTON0);
  CONFIGURE_IO(BUTTON1);

  // Set EXTI inputs for 14/15 to a hardcoded port C
  // TODO(rqou): Clever trick to not hardcode?
  SYSCFG->EXTICR[3] =
    (SYSCFG->EXTICR[3] & ~(SYSCFG_EXTICR4_EXTI14 | SYSCFG_EXTICR4_EXTI15)) |
    (SYSCFG_EXTICR4_EXTI14_PC | SYSCFG_EXTICR4_EXTI15_PC);

  // Set EXTI mask
  EXTI->IMR |=
    (1 << GPIO_PIN(PINDEF_BUTTON0)) | (1 << GPIO_PIN(PINDEF_BUTTON1));

  // Enable both rising and falling edges to trigger this EXTI
  EXTI->RTSR |=
    (1 << GPIO_PIN(PINDEF_BUTTON0)) | (1 << GPIO_PIN(PINDEF_BUTTON1));
  EXTI->FTSR |=
    (1 << GPIO_PIN(PINDEF_BUTTON0)) | (1 << GPIO_PIN(PINDEF_BUTTON1));

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(EXTI15_10_IRQn, 15);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}

int button_driver_get_button_state(int button) {
  if (button == 0) {
    return button0_state;
  } else if (button == 1) {
    return button1_state;
  } else {
    // Error
    return 0;
  }
}

void EXTI15_10_IRQHandler(void) {
  // TODO(rqou): How to share this vector (or if it is even necessary)

  static portTickType button0_last_toggle = 0;
  static portTickType button1_last_toggle = 0;

  portTickType time_now = xTaskGetTickCountFromISR();

  if (EXTI->PR & (1 << GPIO_PIN(PINDEF_BUTTON0))) {
    // Clear pending
    EXTI->PR = (1 << GPIO_PIN(PINDEF_BUTTON0));

    if (time_now - button0_last_toggle >= DEBOUNCE_DELAY) {
      // Invert is necessary because pin idles at 1
      button0_state =
        !(GPIO_BANK(PINDEF_BUTTON0)->IDR & (1 << GPIO_PIN(PINDEF_BUTTON0)));
    }

    button0_last_toggle = time_now;
  }

  if (EXTI->PR & (1 << GPIO_PIN(PINDEF_BUTTON1))) {
    // Clear pending
    EXTI->PR = (1 << GPIO_PIN(PINDEF_BUTTON1));

    if (time_now - button1_last_toggle >= DEBOUNCE_DELAY) {
      // Invert is necessary because pin idles at 1
      button1_state =
        !(GPIO_BANK(PINDEF_BUTTON1)->IDR & (1 << GPIO_PIN(PINDEF_BUTTON1)));
    }

    button1_last_toggle = time_now;
  }
}
