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

#ifndef INC_PINDEF_H_
#define INC_PINDEF_H_

// Board revision to hardware mapping:
//   @ ---> STM32F4DISCOVERY
//   A ---> Yurippe, date code 23 Jan 2014
#define BOARD_REVISION    'A'

// Do not use these macro directly.
// Evaluates to the bitfield in AHB1ENR to turn on the GPIO bank
#define __GPIO_BANK_AHB1ENR(b, p, m, t, s, pu, af) RCC_AHB1ENR_GPIO##b##EN
// Evaluates to the GPIOx structure for the bank the pin is on.
#define __GPIO_BANK(b, p, m, t, s, pu, af)  GPIO##b
// Evaluates to the pin number in the bank.
#define __GPIO_PIN(b, p, m, t, s, pu, af)   p
// Set the proper mode for the particular pin.
#define __SET_GPIO_MODE(b, p, m, t, s, pu, af)  \
  { GPIO##b->MODER = (GPIO##b->MODER & ~(0b11 << (p * 2))) | (m << (p * 2)); }
// Set the proper output type (push-pull vs OD) for the particular pin.
#define __SET_GPIO_TYPE(b, p, m, t, s, pu, af)  \
  { GPIO##b->OTYPER = (GPIO##b->OTYPER & ~(1 << p)) | (t << p); }
// Set the proper speed for the particular pin.
#define __SET_GPIO_SPEED(b, p, m, t, s, pu, af)  \
  { \
    GPIO##b->OSPEEDR =  \
      (GPIO##b->OSPEEDR & ~(0b11 << (p * 2))) | (s << (p * 2)); \
  }
// Set the proper pullup/pulldown for the particular pin.
#define __SET_GPIO_PULLUP(b, p, m, t, s, pu, af)  \
  { GPIO##b->PUPDR = (GPIO##b->PUPDR & ~(0b11 << (p * 2))) | (pu << (p * 2)); }
// Set the proper alternate function for the particular pin.
// Note: The redundant ternaries makes GCC stop issuing shift warnings.
#define __SET_GPIO_AF(b, p, m, t, s, pu, af)  \
  { \
    if (p < 8)  \
    { \
      GPIO##b->AFR[0] = \
          (GPIO##b->AFR[0] & ~(0b1111 << ((p < 8 ? p : 0) * 4)))  \
          | (af << ((p < 8 ? p : 0) * 4)); \
    } \
    else  \
    { \
      GPIO##b->AFR[1] = \
          (GPIO##b->AFR[1] & ~(0b1111 << ((p < 8 ? 0 : p - 8) * 4)))  \
          | (af << ((p < 8 ? 0 : p - 8) * 4)); \
    } \
  }

// These macros should be passed one of the pin definitions PINDEF_x.
// The extra indirection is necessary because macros will expand their
// arguments. This is needed to expand the PINDEF_x macros into two arguments
// for the __x macros
#define GPIO_BANK_AHB1ENR(x)  __GPIO_BANK_AHB1ENR(x)
#define GPIO_BANK(x)          __GPIO_BANK(x)
#define GPIO_PIN(x)           __GPIO_PIN(x)
#define SET_GPIO_MODE(x)      __SET_GPIO_MODE(x)
#define SET_GPIO_TYPE(x)      __SET_GPIO_TYPE(x)
#define SET_GPIO_SPEED(x)     __SET_GPIO_SPEED(x)
#define SET_GPIO_PULLUP(x)    __SET_GPIO_PULLUP(x)
#define SET_GPIO_AF(x)        __SET_GPIO_AF(x)

#define PIN_MODE_INPUT    0
#define PIN_MODE_GPIO     1
#define PIN_MODE_AF       2
#define PIN_MODE_ANALOG   3

#define PIN_TYPE_PUSHPULL   0
#define PIN_TYPE_OPENDRAIN  1

#define PIN_SPEED_LOW   0
#define PIN_SPEED_MED   1
#define PIN_SPEED_FAST  2
#define PIN_SPEED_HIGH  3

#define PIN_PULLUP_NONE 0
#define PIN_PULLUP_PU   1
#define PIN_PULLUP_PD   2

// This macro will call all of the appropriate setup macros.
#define CONFIGURE_IO(ioname)  \
  { \
    SET_GPIO_MODE(PINDEF_##ioname);   \
    SET_GPIO_TYPE(PINDEF_##ioname);   \
    SET_GPIO_SPEED(PINDEF_##ioname);  \
    SET_GPIO_PULLUP(PINDEF_##ioname); \
    SET_GPIO_AF(PINDEF_##ioname);     \
  }

// Bank, number in bank, mode, type, speed, pullup/pulldown, alternate function

#if BOARD_REVISION == '@'

// Debug serial port
#define PINDEF_DEBUG_UART_TX    B, 6, PIN_MODE_AF, PIN_TYPE_PUSHPULL, \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_DEBUG_UART_RX    B, 7, PIN_MODE_AF, PIN_TYPE_PUSHPULL, \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7

// LEDs on main controller
#define PINDEF_YELLOW_LED       D, 13, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_GREEN_LED        D, 12, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_BLUE_LED         D, 15, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_RED_LED          D, 14, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#endif

#if BOARD_REVISION == 'A'

// LEDs on main controller
#define PINDEF_YELLOW_LED       C, 3, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_GREEN_LED        C, 1, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_BLUE_LED         C, 2, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_RED_LED          C, 0, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0

// Buttons on the PCB
// NOTE: This is not the only source of information regarding the buttons.
// You also need to fix up button_driver.c if you move the buttons.
// TODO(rqou): Possibly apply more macro magic to fix this.
#define PINDEF_BUTTON0          C, 14, PIN_MODE_INPUT, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_PU, 0
#define PINDEF_BUTTON1          C, 15, PIN_MODE_INPUT, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_PU, 0

// The legacy I2C channel
#define PINDEF_I2C_SCL          B, 8, PIN_MODE_AF, PIN_TYPE_OPENDRAIN,  \
  PIN_SPEED_LOW, PIN_PULLUP_PU, 4
#define PINDEF_I2C_SDA          B, 9, PIN_MODE_AF, PIN_TYPE_OPENDRAIN,  \
  PIN_SPEED_LOW, PIN_PULLUP_PU, 4

// Smart sensor interfaces
#define PINDEF_SENSOR_CH1_TX    A, 0, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 8
#define PINDEF_SENSOR_CH1_RX    A, 1, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 8
#define PINDEF_SENSOR_CH1_TXE   B, 3, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,   \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_SENSOR_CH2_TX    A, 2, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_SENSOR_CH2_RX    A, 3, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_SENSOR_CH2_TXE   C, 4, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,   \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_SENSOR_CH3_TX    B, 6, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_SENSOR_CH3_RX    B, 7, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_SENSOR_CH3_TXE   C, 5, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,   \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0
#define PINDEF_SENSOR_CH4_TX    C, 6, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 8
#define PINDEF_SENSOR_CH4_RX    C, 7, PIN_MODE_AF, PIN_TYPE_PUSHPULL,     \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 8
#define PINDEF_SENSOR_CH4_TXE   C, 13, PIN_MODE_GPIO, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 0

// Radio
// TODO(rqou): How do we handle switching between either an SPI radio or a UART
// radio? (We currently don't have a SPI radio, but we can)
#define PINDEF_RADIO_TX         B, 10, PIN_MODE_AF, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7
#define PINDEF_RADIO_RX         B, 11, PIN_MODE_AF, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 7

// SPI (used for debugging)
#define PINDEF_SPI_CLK          B, 13, PIN_MODE_AF, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 5
#define PINDEF_SPI_MISO         B, 14, PIN_MODE_AF, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_PD,   5
#define PINDEF_SPI_MOSI         B, 15, PIN_MODE_AF, PIN_TYPE_PUSHPULL,  \
  PIN_SPEED_LOW, PIN_PULLUP_NONE, 5

#endif

// Misc. other definitions

// TODO(rqou): Consolidate the sources of truth for this
#define SYSCLK_FREQ   168000000
#define HCLK_FREQ     SYSCLK_FREQ
#define PCLK1_FREQ    (SYSCLK_FREQ/2)
#define PCLK2_FREQ    (SYSCLK_FREQ/4)

#endif  // INC_PINDEF_H_
