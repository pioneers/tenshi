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

#include "pwm.h"

#include <avr/io.h>

#include "pindef.h"

// WARNING: THIS FILE IS WIP!

void init_pwm(void) {
  // Keep high side driver in reset.
  driver_enable(0);
  DDR(PINDEF_HIGHSIDEENABLE) |= _BV(IO(PINDEF_HIGHSIDEENABLE));

  // dead time
  DT4 = 0;

  // Enable enhanced mode.
  TCCR4E = _BV(ENHC4);

  // Clear count.
  TC4H = 0;
  TCNT4 = 0;

  // Set compare c to max.
  TC4H = 7;
  OCR4C = 0xFF;

  // Enable phase and frequency correct pwm mode.
  TCCR4D = _BV(WGM40);
  // Disable compare d.
  TCCR4C = 0;

  // Prescale by 8.
  TCCR4B = _BV(CS42);
}

void driver_enable(unsigned char enable) {
  if (enable)
    PORT(PINDEF_HIGHSIDEENABLE) |= _BV(IO(PINDEF_HIGHSIDEENABLE));
  else {
    // Reset high side driver.
    PORT(PINDEF_HIGHSIDEENABLE) &= ~(_BV(IO(PINDEF_HIGHSIDEENABLE)));

    // Configure pins as outputs, 0 by default.
    PORTB &= ~(_BV(PB5) | _BV(PB6));
    DDRB |= (_BV(PB5) | _BV(PB6));
    PORTC &= ~(_BV(PC6) | _BV(PC7));
    DDRC |= (_BV(PC6) | _BV(PC7));

    // Enable pwm mode, do not connect pins.
    TCCR4A = _BV(PWM4A) | _BV(PWM4B);
  }
}

void set_pwm_val(unsigned int val) {
  TCCR4E |= _BV(TLOCK4);
  TC4H = (val >> 8) & 7;
  OCR4A = val;
  OCR4B = val;
  TCCR4E &= ~(_BV(TLOCK4));
}

void set_sign_magnitude_go_brake_fwd(void) {
  // Fix side b to have top off and bottom on.
  PORTC = (PORTC & ~(_BV(PC7))) | _BV(PC6);
  // Clear the other output.
  PORTB &= ~(_BV(PB5) | _BV(PB6));
  // Make side b output
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Make both pins on side a output
  DDRB |= (_BV(PB6) | _BV(PB5));
  // Invert output
  TCCR4B |= _BV(PWM4X);
  // Connect side a to pwm.
  TCCR4A = _BV(COM4B0) | _BV(PWM4A) | _BV(PWM4B);
}

void set_sign_magnitude_go_brake_bck(void) {
  // Fix side a to have top off and bottom on.
  PORTB = (PORTB & ~(_BV(PB5))) | _BV(PB6);
  // Clear the other output.
  PORTC &= ~(_BV(PC6) | _BV(PC7));
  // Make all outputs
  DDRB |= (_BV(PB5) | _BV(PB6));
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Do not invert output
  TCCR4B &= ~(_BV(PWM4X));
  // Connect side b to pwm.
  TCCR4A = _BV(COM4A0) | _BV(PWM4A) | _BV(PWM4B);
}

void set_sign_magnitude_go_coast_fwd(void) {
  // Fix side b to have top off and bottom on.
  PORTC = (PORTC & ~(_BV(PC7))) | _BV(PC6);
  // Clear the other output.
  PORTB &= ~(_BV(PB5) | _BV(PB6));
  // Make side b output
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Make only inverted pin on side a output
  DDRB = ((DDRB & ~(_BV(PB6))) | _BV(PB5));
  // Invert output
  TCCR4B |= _BV(PWM4X);
  // Connect side a to pwm.
  TCCR4A = _BV(COM4B0) | _BV(PWM4A) | _BV(PWM4B);
}

void set_sign_magnitude_go_coast_bck(void) {
  // Fix side a to have top off and bottom on.
  PORTB = (PORTB & ~(_BV(PB5))) | _BV(PB6);
  // Clear the other output.
  PORTC &= ~(_BV(PC6) | _BV(PC7));
  // Make all outputs
  DDRB |= (_BV(PB5) | _BV(PB6));
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Do not invert output
  TCCR4B &= ~(_BV(PWM4X));
  // Connect side b HIGH ONLY to pwm.
  TCCR4A = _BV(COM4A1) | _BV(PWM4A) | _BV(PWM4B);
}

void set_locked_antiphase(void) {
  // Make all outputs
  DDRB |= (_BV(PB5) | _BV(PB6));
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Invert output
  TCCR4B |= _BV(PWM4X);
  // Connect side a and side b to pwm.
  TCCR4A = _BV(COM4A0) | _BV(COM4B0) | _BV(PWM4A) | _BV(PWM4B);
}

void set_controlled_brake(void) {
  // Fix side b to have top off and bottom on.
  PORTC = (PORTC & ~(_BV(PC7))) | _BV(PC6);
  // Clear the other output.
  PORTB &= ~(_BV(PB5) | _BV(PB6));
  // Make side b output
  DDRC |= (_BV(PC6) | _BV(PC7));
  // Make LOW pins on side a output
  DDRB = ((DDRB & ~(_BV(PB5))) | _BV(PB6));
  // Connect side a to pwm.
  TCCR4A = _BV(COM4B0) | _BV(PWM4A) | _BV(PWM4B);
}
