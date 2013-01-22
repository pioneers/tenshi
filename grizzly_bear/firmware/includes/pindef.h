#ifndef PINDEF_H
#define PINDEF_H

#define BOARD_REVISION          'D'

// The following macros are used for pin abstractions.
// The ones with two underscores should not be directly.
#define __IO(x, y)              P##x##y
#define __DDR(x, y)             DDR##x
#define __PIN(x, y)             PIN##x
#define __PORT(x, y)            PORT##x

// The following macros will expand to Pxn/DDRx/PINx/PORTx defines.
// They should be passed one of th pin definitions PINDEF_x.
// The extra indirection is necessary because macros will expand their
// arguments. This is needed to expand the PINDEF_x macros into two arguments
// for the __x macros
#define IO(x)                   __IO(x)
#define DDR(x)                  __DDR(x)
#define PIN(x)                  __PIN(x)
#define PORT(x)                 __PORT(x)

// dip switch address select
#define PINDEF_DIP1             F, 1
#define PINDEF_DIP2             F, 4
#define PINDEF_DIP3             F, 5
#if BOARD_REVISION < 'D'
#define PINDEF_DIP4             D, 3
#else
#define PINDEF_DIP4             B, 7
#endif
#define PINDEF_DIP5             F, 6
#define PINDEF_DIP6             F, 7

// rotary encoder
// These are REQUIRED to be on the same port bank
#define PINDEF_ENCA             B, 0
#define PINDEF_ENCB             B, 4

// high side driver endable
#define PINDEF_HIGHSIDEENABLE   E, 2

#define PINDEF_LEDGRN           D, 6
#define PINDEF_LEDRED           D, 4

#endif
