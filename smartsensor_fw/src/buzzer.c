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

// This file has battery buzzer related functions

#include "inc/buzzer.h"

// Private global variables

// Private helper functions
int adc_read(void);

// Public functions called from main.c
void initBuzzer() {
  // Does everything to set up the analog stuffs.
  // Turn on pin PC1 (which maps to IN0)
  ADMUX |= (1 << MUX3) | (1 << MUX1);
  // Enable the ADC and set the division factor between
  // the system clock frequency and the input clock to the ADC.
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}
void activeBuzzerRec(uint8_t *data, uint8_t len, uint8_t inband) {
}
void activeBuzzerSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  // TODO(cduck): Write this function
}

// Private helper functions

// Taken from http://www.adnbr.co.uk/articles/adc-and-pwm-basics
// by Sumita because I can't code.
// It's for the ATtiny13, though.
/*int adc_read(void) {
    // I actually use this one. It's the analog in version of DIGITAL_READ.
    // Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait for it to finish
    while (ADCSRA & (1 << ADSC)) {}

    return ADC;  // For 10-bit resolution
}*/
