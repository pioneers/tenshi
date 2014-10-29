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

#include "inc/smartsensor/common.h"
#include "inc/smartsensor_utility.h"

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
