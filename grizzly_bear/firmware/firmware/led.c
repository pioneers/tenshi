#include "led.h"

#include <avr/io.h>

#include "pindef.h"

void init_led() {
  // Set the the green and red led pins to output.
  LED_PORT_MODE |= _BV(PINDEF_LEDGRN) | _BV(PINDEF_LEDRED);
}

void set_green_led(unsigned char on) {
  if (on) {
    LED_PORT |= _BV(PINDEF_LEDGRN);
  }
  else {
    LED_PORT &= ~(_BV(PINDEF_LEDGRN));
  }
}

void set_red_led(unsigned char on) {
  if (on) {
    LED_PORT |= _BV(PINDEF_LEDRED);
  }
  else {
    LED_PORT &= ~(_BV(PINDEF_LEDRED));
  }
}

void set_all_leds(unsigned char on) {
  set_green_led(on);
  set_red_led(on);
}
