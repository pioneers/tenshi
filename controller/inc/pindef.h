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
#define __SET_GPIO_AF(b, p, m, t, s, pu, af)  \
  { \
    if (p < 8)  \
    { \
      GPIO##b->AFR[0] = \
          (GPIO##b->AFR[0] & ~(0b1111 << (p * 4))) | (af << (p * 4)); \
    } \
    else  \
    { \
      GPIO##b->AFR[1] = \
          (GPIO##b->AFR[1] & ~(0b1111 << ((p - 8) * 4)))  \
          | (af << ((p - 8) * 4)); \
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
#endif

#endif  // INC_PINDEF_H_
