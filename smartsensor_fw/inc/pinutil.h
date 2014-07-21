
#ifndef INC_PINUTIL_H_
#define INC_PINUTIL_H_

#include <avr/io.h>


#define PIN_DEFINITION(PORT_LETTER, PIN_NUMBER) \
          PORT##PORT_LETTER, P##PORT_LETTER##PIN_NUMBER, \
          PIN##PORT_LETTER, PIN##PORT_LETTER##PIN_NUMBER, \
          DDR##PORT_LETTER, DDR##PORT_LETTER##PIN_NUMBER, \
          PUE##PORT_LETTER, PIN##PORT_LETTER##PIN_NUMBER

// Use one of the above pin definitions as an argument to these macros
#define DIGITAL_SET_OUT(x) __DIGITAL_SET_OUT(x)
#define __DIGITAL_SET_OUT(a, b, c, d, e, f, g, h) \
          e |= (1 << f)
#define DIGITAL_SET_IN(x) __DIGITAL_SET_IN(x)
#define __DIGITAL_SET_IN(a, b, c, d, e, f, g, h) \
          e &= ~(1 << f)

#define DIGITAL_SET_HIGH(x) __DIGITAL_SET_HIGH(x)
#define __DIGITAL_SET_HIGH(a, b, c, d, e, f, g, h) \
          a |= (1 << b)
#define DIGITAL_SET_LOW(x) __DIGITAL_SET_LOW(x)
#define __DIGITAL_SET_LOW(a, b, c, d, e, f, g, h) \
          a &= ~(1 << b)
#define DIGITAL_TOGGLE(x) __DIGITAL_TOGGLE(x)
#define __DIGITAL_TOGGLE(a, b, c, d, e, f, g, h) \
          a ^= (1 << b)
#define DIGITAL_SET(x, flag) __DIGITAL_SET(x, flag)
#define __DIGITAL_SET(a, b, c, d, e, f, g, h, flag) \
          a = (a & ~(1 << b)) | ((!!(flag)) << b)

#define DIGITAL_PULLUP_ON(x) __DIGITAL_PULLUP_ON(x)
#define __DIGITAL_PULLUP_ON(a, b, c, d, e, f, g, h) \
          g |= (1 << h)
#define DIGITAL_PULLUP_OFF(x) __DIGITAL_PULLUP_OFF(x)
#define __DIGITAL_PULLUP_OFF(a, b, c, d, e, f, g, h) \
          g &= ~(1 << h)
#define DIGITAL_SET_PULLUP(x, flag) __DIGITAL_SET_PULLUP(x, flag)
#define __DIGITAL_SET_PULLUP(a, b, c, d, e, f, g, h, flag) \
          g = (g & ~(1 << h)) | ((!!(flag)) << h)

#define DIGITAL_READ(x) __DIGITAL_READ(x)
#define __DIGITAL_READ(a, b, c, d, e, f, g, h) \
          (!!(c & (1 << d)))



#endif  // INC_PINUTIL_H_
