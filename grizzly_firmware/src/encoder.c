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

#include "inc/encoder.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "inc/pindef.h"

DECLARE_I2C_REGISTER_C(int32_t, encoder_count);

void init_encoder() {
  DDR(PINDEF_ENCA) &= ~(_BV(IO(PINDEF_ENCA)) | _BV(IO(PINDEF_ENCB)));
  // Init is called before interrupts are enabled.
  set_encoder_count_dangerous(0);
  // Set up interrupt.
  PCMSK0 = _BV(PCINT0) | _BV(PCINT4);
  // Just in case.
  PCIFR = _BV(PCIF0);
  // Enable.
  PCICR = _BV(PCIE0);
}

// Indexed by (old_state << 2) | (new_state).
// Zero entries are somehow invalid (two transition or no transitions).
// When going forward, the signals look like:
//  a ___|‾‾‾|___|‾‾‾|___|‾‾‾|___
//  b   ___|‾‾‾|___|‾‾‾|___|‾‾‾|___
// Interrupts happen AFTER transitions, so the "forward" transitions are:
//   00 to 10, 10 to 11, 11 to 01, and 01 to 00
// When going backwards, the signals look like:
//   a   ___|‾‾‾|___|‾‾‾|___|‾‾‾|___
//   b ___|‾‾‾|___|‾‾‾|___|‾‾‾|___
// Interrupts happen AFTER transitions, so the "backward" transitions are:
//  00 to 01, 01 to 11, 11 to 10, and 10 to 00
const signed char encoder_transition_table[] = {
  0,
  -1,
  1,
  0,
  1,
  0,
  0,
  -1,
  -1,
  0,
  0,
  1,
  0,
  1,
  -1,
  0,
};

// Interrupt for encoder processing
// This gets called every time one of pin A or pin B changes
// (but you don't know which).
ISR(PCINT0_vect) {
  unsigned char ioport_copy = PIN(PINDEF_ENCA);
  static unsigned char old_state = 0;
  unsigned char new_state = 0;
  if (ioport_copy & _BV(IO(PINDEF_ENCA)))
    new_state |= 0b10;
  if (ioport_copy & _BV(IO(PINDEF_ENCB)))
    new_state |= 0b01;
  set_encoder_count_dangerous(get_encoder_count_dangerous() +
      encoder_transition_table[old_state << 2 | new_state]);
  old_state = new_state;
}
