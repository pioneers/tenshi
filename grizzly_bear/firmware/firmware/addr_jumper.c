#include "addr_jumper.h"

#include <avr/io.h>

#include "pindef.h"

static void all_off(void) {
  // Enable all the pull ups, all set as inputs.
  DDR(PINDEF_DIP1) &= ~(_BV(IO(PINDEF_DIP1)));
  DDR(PINDEF_DIP2) &= ~(_BV(IO(PINDEF_DIP2)));
  DDR(PINDEF_DIP3) &= ~(_BV(IO(PINDEF_DIP3)));
  DDR(PINDEF_DIP4) &= ~(_BV(IO(PINDEF_DIP4)));
  DDR(PINDEF_DIP5) &= ~(_BV(IO(PINDEF_DIP5)));
  DDR(PINDEF_DIP6) &= ~(_BV(IO(PINDEF_DIP6)));

  PORT(PINDEF_DIP1) |= (_BV(IO(PINDEF_DIP1)));
  PORT(PINDEF_DIP2) |= (_BV(IO(PINDEF_DIP2)));
  PORT(PINDEF_DIP3) |= (_BV(IO(PINDEF_DIP3)));
  PORT(PINDEF_DIP4) |= (_BV(IO(PINDEF_DIP4)));
  PORT(PINDEF_DIP5) |= (_BV(IO(PINDEF_DIP5)));
  PORT(PINDEF_DIP6) |= (_BV(IO(PINDEF_DIP6)));
}

// This is needed because one clock cycle is not always enough for the pin
// output change to actually get detected because of capacitance or inducance or
// something. (The first if statement is one opcode after the previous pin set
// without this.)
static inline void short_delay(void) {
  for (unsigned char i = 0; i < 100; i++)
    asm volatile(""); // Prevent optimizing loop away.
}

// This function scans the pin headers and determines what address should be
// set. It is designed to have 7 possible addresses and use 1 jumper.
// Consult the wiki for the possible combinations.
unsigned char determine_addr(void) {
  // Configure pin 5 (top left) as output, set it to 0.
  all_off();
  DDR(PINDEF_DIP5) |= _BV(IO(PINDEF_DIP5));
  PORT(PINDEF_DIP5) &= ~(_BV(IO(PINDEF_DIP5)));

  short_delay();

  if (!(PIN(PINDEF_DIP6) & _BV(IO(PINDEF_DIP6)))) {
    all_off();
    return ADDR_COMBO_1;
  }
  if (!(PIN(PINDEF_DIP3) & _BV(IO(PINDEF_DIP3)))) {
    all_off();
    return ADDR_COMBO_4;
  }
  // Configure pin 4 (bottom middle) as output, set it to 0
  all_off();
  DDR(PINDEF_DIP4) |= _BV(IO(PINDEF_DIP4));
  PORT(PINDEF_DIP4) &= ~(_BV(IO(PINDEF_DIP4)));

  short_delay();

  if (!(PIN(PINDEF_DIP3) & _BV(IO(PINDEF_DIP3)))) {
    all_off();
    return ADDR_COMBO_2;
  }
  if (!(PIN(PINDEF_DIP6) & _BV(IO(PINDEF_DIP6)))) {
    all_off();
    return ADDR_COMBO_5;
  }
  // Configure pin 1 (top right) as output, set it to 0.
  all_off();
  DDR(PINDEF_DIP1) |= _BV(IO(PINDEF_DIP1));
  PORT(PINDEF_DIP1) &= ~(_BV(IO(PINDEF_DIP1)));

  short_delay();

  if (!(PIN(PINDEF_DIP2) & _BV(IO(PINDEF_DIP2)))) {
    all_off();
    return ADDR_COMBO_3;
  }
  if (!(PIN(PINDEF_DIP3) & _BV(IO(PINDEF_DIP3)))) {
    all_off();
    return ADDR_COMBO_6;
  }

  all_off();
  return ADDR_COMBO_7;
}
