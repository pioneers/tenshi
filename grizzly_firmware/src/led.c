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

#include "inc/led.h"

#include <avr/io.h>

#include "inc/pindef.h"

void init_led() {
  // Set the the green and red led pins to output.
  DDR(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDGRN));
  DDR(PINDEF_LEDRED) |= _BV(IO(PINDEF_LEDRED));
}

void set_green_led(unsigned char on) {
  if (on) {
    PORT(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDGRN));
  } else {
    PORT(PINDEF_LEDRED) &= ~(_BV(IO(PINDEF_LEDGRN)));
  }
}

void set_red_led(unsigned char on) {
  if (on) {
    PORT(PINDEF_LEDGRN) |= _BV(IO(PINDEF_LEDRED));
  } else {
    PORT(PINDEF_LEDRED) &= ~(_BV(IO(PINDEF_LEDRED)));
  }
}

void set_all_leds(unsigned char on) {
  set_green_led(on);
  set_red_led(on);
}
