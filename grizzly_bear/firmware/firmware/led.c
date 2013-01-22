#include "led.h"

#include <avr/io.h>

#include "pindef.h"

void init_led() {
  // Set the the green and red led pins to output.
  DDR(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDGRN));
  DDR(PINDEF_LEDRED) |= _BV(IO(PINDEF_LEDRED));
}

void set_green_led(unsigned char on) {
  if (on) {
    PORT(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDGRN));
  }
  else {
    PORT(PINDEF_LEDRED) &= ~(_BV(IO(PINDEF_LEDGRN)));
  }
}

void set_red_led(unsigned char on) {
  if (on) {
    PORT(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDRED));
  }
  else {
    PORT(PINDEF_LEDRED) &= ~(_BV(IO(PINDEF_LEDRED)));
  }
}

void set_all_leds(unsigned char on) {
  set_green_led(on);
  set_red_led(on);
}
