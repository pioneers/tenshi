// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#include "inc/button_driver.h"

#include "inc/FreeRTOS.h"
#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/pindef.h"
#include "inc/task.h"

static volatile int button0_state = 0;
static volatile int button1_state = 0;

static volatile int button0_actual_state = 0;
static volatile int button1_actual_state = 0;

static volatile portTickType button0_last_toggle = 0;
static volatile portTickType button1_last_toggle = 0;

#define DEBOUNCE_DELAY  (50 / portTICK_RATE_MS)

void button_driver_update_debounce();


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
  button_driver_update_debounce();

  if (button == 0) {
    return button0_state;
  } else if (button == 1) {
    return button1_state;
  } else {
    // Error
    return 0;
  }
}

void button_driver_update_debounce() {
  portTickType time_now = xTaskGetTickCountFromISR();
  if (time_now - button0_last_toggle >= DEBOUNCE_DELAY) {
    button0_state = button0_actual_state;
  }
  if (time_now - button1_last_toggle >= DEBOUNCE_DELAY) {
    button1_state = button1_actual_state;
  }
}

void EXTI15_10_IRQHandler(void) {
  // TODO(rqou): How to share this vector (or if it is even necessary)

  portTickType time_now = xTaskGetTickCountFromISR();

  if (EXTI->PR & (1 << GPIO_PIN(PINDEF_BUTTON0))) {
    // Clear pending
    EXTI->PR = (1 << GPIO_PIN(PINDEF_BUTTON0));
    // Invert is necessary because pin idles at 1
    int newState =
      !(GPIO_BANK(PINDEF_BUTTON0)->IDR & (1 << GPIO_PIN(PINDEF_BUTTON0)));

    if (time_now - button0_last_toggle >= DEBOUNCE_DELAY) {
      button0_state = newState;
    }

    button0_last_toggle = time_now;
    button0_actual_state = newState;
  }

  if (EXTI->PR & (1 << GPIO_PIN(PINDEF_BUTTON1))) {
    // Clear pending
    EXTI->PR = (1 << GPIO_PIN(PINDEF_BUTTON1));
    // Invert is necessary because pin idles at 1
    int newState =
      !(GPIO_BANK(PINDEF_BUTTON1)->IDR & (1 << GPIO_PIN(PINDEF_BUTTON1)));

    if (time_now - button1_last_toggle >= DEBOUNCE_DELAY) {
      // Invert is necessary because pin idles at 1
      button1_state = newState;
    }

    button1_last_toggle = time_now;
    button1_actual_state = newState;
  }
}
