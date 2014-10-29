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

#include <avr/interrupt.h>
#include "inc/buzzer.h"
#include "inc/smartsensor_utility.h"
#include "inc/pindef.h"

// Private global variables

// Private helper functions

// Public functions called from main.c
void initBuzzer() {
  // Does everything to set up the analog stuffs.
  // Turn on pin PC1 (which maps to IN0)
  ADMUX |= (1 << MUX3) | (1 << MUX1);
  #if F_CPU != 8000000
  #error Clock speed not correct
  #endif
  // Enable the ADC and set the division factor between
  // the system clock frequency and the input clock to the ADC.
  // Division factor: 111 = 128
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);

  // Set PA6 pin as output in order to see PWM signal
  // Set pulse width to 1/2 of entire duration.
  OCR1BH = 0x00;
  OCR1BL = 0x7F;
  DIGITAL_SET_OUT(PWM0);
  
  // Testing
  DIGITAL_SET_OUT(PWM1);
  DIGITAL_SET_HIGH(PWM1);
  // End Testing 

  // Set PWM to Fast PWM Mode Operation, with non-inverting PWM
  TCCR1A = 0x21;
  TCCR1B = 0x0B;

  // Enable Timer/Counter1 Overflow Interrupt
  TIMSK = 0x80;

  sei();
}

void activeBuzzerRec(uint8_t *data, uint8_t len, uint8_t inband) {
}

void activeBuzzerSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  // TODO(cduck): Write this function
}


// Interrupt that checks whether to buzz the battery
ISR(TIMER1_OVF_vect){
  static unsigned char counter = 0;

  DIGITAL_TOGGLE(PWM1);
  
  if (counter > 40){
    if ((TCCR1A >> 4) != 0x00){
      TCCR1A = 0x01;
    }else{
      TCCR1A = 0x21;
    }
    counter = 0;
  }
  counter++;
}

// Private helper functions

