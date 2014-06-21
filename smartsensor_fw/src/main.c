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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

int main() {
  int x = 1;
  /*
    Define directions for port pins
  */
  // portA not used right now so this should do nothing
  PORTA = (0 << PA0) | (0 << PA1) | (0 << PA2);
  DDRA = (1 << DDRA0) | (1 << DDRA1) | (1 << DDRA2);

  // B3 should be IN1
  PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
  DDRB = (1 << DDRB0) | (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);

  // C1 sould be IN0
  PORTC = (0 << PC0) | (0 << PC1) | (0 << PC2);
  DDRC = (1 << DDRC0) | (1 << DDRC1) | (1 << DDRC2);

  while (x) {
    PORTB = PORTB ^ (1 << PB3);
  }
}
