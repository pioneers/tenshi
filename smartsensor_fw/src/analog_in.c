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

// This file has analog_in io smart sensor related functions

#include "inc/analog_in.h"

// Private global variables

// Private helper functions

// Public functions called from main.c
void initAnalogIn() {
  // Does everything to set up the analog stuffs.
  // Turn on pin PC1 (which maps to IN0)
  ADMUX |= (1 << MUX3) | (1 << MUX1);
  // Enable the ACD and set the division factor between
  // the system clock frequency and the input clock to the ADC.
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}
void activeAnalogInRec(uint8_t *data, uint8_t len, uint8_t inband) {
  // Casey told me to leave this empty. So hi.
  // When the ss receives a command from the control, this gets called.
  // But I don't need it here because it's reading a pot so
  // there's no information coming from the controller.
  // I'll need this for the analog out code, though.
}
void activeAnalogInSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  *outLen = 8;
  int val0 = adc_read((1 << MUX3) | (1 << MUX1));  // 10 bit resolution
  int val1 = adc_read((1 << MUX3) | (1 << MUX1));
  int val2 = adc_read((1 << MUX3) | (1 << MUX1));
  int val3 = adc_read((1 << MUX3) | (1 << MUX1));
  outData[0] = val0;
  outData[1] = val0 >> 8;
  outData[2] = val1;
  outData[3] = val1 >> 8;
  outData[4] = val2;
  outData[5] = val2 >> 8;
  outData[6] = val3;
  outData[7] = val3 >> 8;
}

// Private helper functions

// Taken from http://www.adnbr.co.uk/articles/adc-and-pwm-basics
// by Sumita because I can't code.
// It's for the ATtiny13, though.
int adc_read(uint8_t mux) {
    // Set the read pin
    ADMUX = mux;

    // I actually use this one. It's the analog in version of DIGITAL_READ.
    // Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait for it to finish
    while (ADCSRA & (1 << ADSC)) {}

    return ADC;  // For 10-bit resolution
}
