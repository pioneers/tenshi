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

#include "inc/led_driver.h"

#include "inc/FreeRTOS.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/task.h"
#include "inc/button_driver.h"

static volatile uint8_t led_driver_current_mode = 0;
static volatile uint8_t led_driver_fixed_pattern = 0;  // The states of the
                                                       // LEDs that don't flash
static volatile uint8_t led_driver_fixed_mask = 0;  // The LEDs that don't flash

#define LED_YELLOW    0b0001
#define LED_BLUE      0b0010
#define LED_GREEN     0b0100
#define LED_RED       0b1000

#define LED_TICK_TIME 100  // 250

typedef struct tag_led_driver_pattern_entry {
  uint8_t led_states;
  uint8_t num_ticks;
} led_driver_pattern_entry;

typedef struct tag_led_driver_pattern {
  uint8_t len;
  const led_driver_pattern_entry *entries;
} led_driver_pattern;

static const led_driver_pattern const driver_patterns[] = {
  {
    // PATTERN_DEFAULT_CHASER
    // Each LED on for 1 second
    .len = 4,
    .entries =
    (const led_driver_pattern_entry[]) {
      {
        .led_states = LED_YELLOW,
        .num_ticks = 4,
      }, {
        .led_states = LED_BLUE,
        .num_ticks = 4,
      }, {
        .led_states = LED_GREEN,
        .num_ticks = 4,
      }, {
        .led_states = LED_RED,
        .num_ticks = 4,
      },
    },
  }, {
    // PATTERN_BACK_AND_FORTH
    // LEDs light up back and forth for 1 tick each
    // Note that to make looping smooth red and yellow are not repeated.
    .len = 6,
    .entries =
    (const led_driver_pattern_entry[]) {
      {
        .led_states = LED_YELLOW,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE,
        .num_ticks = 1,
      }, {
        .led_states = LED_GREEN,
        .num_ticks = 1,
      }, {
        .led_states = LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_GREEN,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE,
        .num_ticks = 1,
      },
    },
  }, {
    // PATTERN_JUST_RED
    // Just red flashes
    .len = 2,
    .entries =
    (const led_driver_pattern_entry[]) {
      {
        .led_states = LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = 0,
        .num_ticks = 1,
      },
    },
  }, {
    // PATTERN_ENUMERATING
    // Flashes pairs of LEDs during sensor enumeration
    .len = 2,
    .entries =
    (const led_driver_pattern_entry[]) {
      {
        .led_states = LED_YELLOW | LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE | LED_GREEN,
        .num_ticks = 1,
      },
    },
  }, {
    // PATTERN_RUNTIME_ERROR
    // Flashes all LEDs on and off
    .len = 2,
    .entries =
    (led_driver_pattern_entry[]) {
      {
        .led_states = LED_YELLOW | LED_BLUE | LED_GREEN | LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = 0,
        .num_ticks = 1,
      },
    },
  }, {
    // PATTERN_RUNTIME_DISABLED
    // Single LED chase with pause
    .len = 5,
    .entries =
    (led_driver_pattern_entry[]) {
      {
        .led_states = LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_GREEN,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW,
        .num_ticks = 1,
      }, {
        .led_states = 0,
        .num_ticks = 2,
      },
    },
  }, {
    // PATTERN_RUNTIME_AUTONOMOUS
    // Double LED chase with pause
    .len = 6,
    .entries =
    (led_driver_pattern_entry[]) {
      {
        .led_states = LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_GREEN | LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE | LED_GREEN,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW | LED_BLUE,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW,
        .num_ticks = 1,
      }, {
        .led_states = 0,
        .num_ticks = 2,
      },
    },
  }, {
    // PATTERN_RUNTIME_TELEOP
    // Triple LED chase with pause
    .len = 7,
    .entries =
    (led_driver_pattern_entry[]) {
      {
        .led_states = LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_GREEN | LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_BLUE | LED_GREEN | LED_RED,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW | LED_BLUE | LED_GREEN,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW | LED_BLUE,
        .num_ticks = 1,
      }, {
        .led_states = LED_YELLOW,
        .num_ticks = 1,
      }, {
        .led_states = 0,
        .num_ticks = 2,
      },
    },
  },
};

// Enable GPIO banks and configure I/O mode
static void led_driver_init_hw(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_RED_LED);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_GREEN_LED);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_BLUE_LED);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_YELLOW_LED);

  // Set on board LED GPIO output
  CONFIGURE_IO(RED_LED);
  CONFIGURE_IO(GREEN_LED);
  CONFIGURE_IO(BLUE_LED);
  CONFIGURE_IO(YELLOW_LED);
}

void led_driver_panic(void) {
  led_driver_init_hw();

  // Turn on all the LEDs
  GPIO_BANK(PINDEF_RED_LED)->BSRRL =
    (1 << GPIO_PIN(PINDEF_RED_LED));
  GPIO_BANK(PINDEF_GREEN_LED)->BSRRL =
    (1 << GPIO_PIN(PINDEF_GREEN_LED));
  GPIO_BANK(PINDEF_BLUE_LED)->BSRRL =
    (1 << GPIO_PIN(PINDEF_BLUE_LED));
  GPIO_BANK(PINDEF_YELLOW_LED)->BSRRL =
    (1 << GPIO_PIN(PINDEF_YELLOW_LED));
}

static void update_hardware(uint8_t change_mask, uint8_t led_states) {
  if (change_mask & LED_YELLOW) {
    if (led_states & LED_YELLOW) {
      GPIO_BANK(PINDEF_YELLOW_LED)->BSRRL =
        (1 << GPIO_PIN(PINDEF_YELLOW_LED));
    } else {
      GPIO_BANK(PINDEF_YELLOW_LED)->BSRRH =
        (1 << GPIO_PIN(PINDEF_YELLOW_LED));
    }
  }
  if (change_mask & LED_BLUE) {
    if (led_states & LED_BLUE) {
      GPIO_BANK(PINDEF_BLUE_LED)->BSRRL =
        (1 << GPIO_PIN(PINDEF_BLUE_LED));
    } else {
      GPIO_BANK(PINDEF_BLUE_LED)->BSRRH =
        (1 << GPIO_PIN(PINDEF_BLUE_LED));
    }
  }
  if (change_mask & LED_GREEN) {
    if (led_states & LED_GREEN) {
      GPIO_BANK(PINDEF_GREEN_LED)->BSRRL =
        (1 << GPIO_PIN(PINDEF_GREEN_LED));
    } else {
      GPIO_BANK(PINDEF_GREEN_LED)->BSRRH =
        (1 << GPIO_PIN(PINDEF_GREEN_LED));
    }
  }
  if (change_mask & LED_RED) {
    if (led_states & LED_RED) {
      GPIO_BANK(PINDEF_RED_LED)->BSRRL =
        (1 << GPIO_PIN(PINDEF_RED_LED));
    } else {
      GPIO_BANK(PINDEF_RED_LED)->BSRRH =
        (1 << GPIO_PIN(PINDEF_RED_LED));
    }
  }
}

static portTASK_FUNCTION_PROTO(led_driver_task, pvParameters) {
  (void) pvParameters;

  uint8_t current_led_pattern = 0;
  uint8_t prev_led_pattern = 0;
  uint8_t pattern_step_index = 0;
  uint8_t current_step_ticks = 0;

  while (1) {
    current_led_pattern = led_driver_current_mode;

    // New pattern --> reinit
    if (current_led_pattern != prev_led_pattern) {
      prev_led_pattern = current_led_pattern;
      pattern_step_index = 0;
      current_step_ticks = 0;
    }

    // Set/clear the appropriate LEDs
    uint8_t led_states = driver_patterns[current_led_pattern]
      .entries[pattern_step_index].led_states;
    led_states &= ~led_driver_fixed_mask;
    led_states |= led_driver_fixed_pattern & led_driver_fixed_mask;
    if (button_driver_get_button_state(0))
      led_states = led_driver_fixed_pattern;
    if (button_driver_get_button_state(1)) led_states = ~led_states;

    update_hardware(0xff, led_states);

    // Increment tick/advance step
    if (++current_step_ticks ==
      driver_patterns[current_led_pattern]
      .entries[pattern_step_index].num_ticks) {
      current_step_ticks = 0;
      // Increment/reset step
      if (++pattern_step_index == driver_patterns[current_led_pattern].len) {
        pattern_step_index = 0;
      }
    }

    // Sleep for 1/4 second (tick time for LED patterns)
    vTaskDelay(LED_TICK_TIME / portTICK_RATE_MS);
  }
}

void led_driver_init(void) {
  led_driver_init_hw();

  xTaskCreate(led_driver_task, "LEDs", 256, NULL, tskIDLE_PRIORITY, NULL);
}

void led_driver_set_mode(uint8_t mode) {
  led_driver_current_mode = mode;
}

uint8_t led_driver_get_mode(void) {
  return led_driver_current_mode;
}

// TODO(cduck): Make thread safe
void led_driver_set_fixed(uint8_t pattern, uint8_t mask) {
  led_driver_fixed_pattern = pattern;
  led_driver_fixed_mask = mask;
  update_hardware(led_driver_fixed_mask, led_driver_fixed_pattern);
}
uint8_t led_driver_get_fixed_pattern() {
  return led_driver_fixed_pattern;
}
uint8_t led_driver_get_fixed_mask() {
  return led_driver_fixed_mask;
}
