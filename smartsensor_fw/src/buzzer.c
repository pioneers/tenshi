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
int batteryUnsafe();

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
  OCR1BL = 0x7F;
  DIGITAL_SET_OUT(PWM0);
  // DIGITAL_SET is also ANALOG_SET apparently...
  DIGITAL_SET_IN(IN1);
  DIGITAL_SET_IN(IN2);
  DIGITAL_SET_IN(IN3);

  // Testing
  DIGITAL_SET_OUT(PWM1);
  DIGITAL_SET_HIGH(PWM1);
  // End Testing

  // Set PWM to Fast PWM Mode Operation, with non-inverting PWM
  // Clear OC1B on Compare Match
  // Clock prescaler = 1/64
  TCCR1A = (1 << COM1B1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);

  // Enable Timer/Counter1 Overflow Interrupt
  TIMSK = (1 << TOIE1);

  sei();
}

void activeBuzzerRec(uint8_t *data, uint8_t len, uint8_t inband) {
}

void activeBuzzerSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  // outData[0] = 1 if safe, 0 if unsafe
  // outData[x-1:x] = IO[x/2] voltage * 17
  *outLen = 7;

  // Read input from pin. Use Vcc as analog reference
  // Pins PB1,2,3 correspond to IO1,2,3
  uint8_t ref = 0x0F;  //
  uint8_t muxPB1 = (ref & A_IN1);  // PB1
  uint8_t muxPB2 = (ref & A_IN2);  // PB2
  uint8_t muxPB3 = (ref & A_IN3);  // PB3
  int IO1 = adc_read(muxPB1) / 6;
  int IO2 = adc_read(muxPB2) / 3;
  int IO3 = adc_read(muxPB3) / 2;

  outData[0] = 1 - batteryUnsafe();
  outData[1] = IO1 >> 8;
  outData[2] = IO1;
  outData[3] = IO2 >> 8;
  outData[4] = IO2;
  outData[5] = IO3 >> 8;
  outData[6] = IO3;
}


// Interrupt that checks whether to buzz the battery
ISR(TIMER1_OVF_vect) {
  static unsigned char counter = 0;

  DIGITAL_TOGGLE(PWM1);  // Testing


  // Run this code once every 40 interrupts
  if (counter > 40) {
    if (batteryUnsafe()) {
      TCCR1A = (1 << COM1B1) | (1 << WGM10);  // Turn on PWM
    } else {
      TCCR1A = (1 << WGM10);  // Turn off PWM
    }
    counter = 0;
  }
  counter++;
}

// Private helper functions
int batteryUnsafe() {
  // Returns 1 if the battery is unsafe, 0 if safe.

  // Read input from pin. Use Vcc as analog reference
  // Pins PB1,2,3 correspond to IO1,2,3
  uint8_t ref = 0x0F;  //
  uint8_t muxPB1 = (ref & A_IN1);  // PB1
  uint8_t muxPB2 = (ref & A_IN2);  // PB2
  uint8_t muxPB3 = (ref & A_IN3);  // PB3

  // Return value ranges from 0x000 to 0x3FF;
  // Division converts everything to same scaling
  // If voltage = V, input I = (V*1024)/(V_ref*12) ~= 17*V
  int IO1 = adc_read(muxPB1) / 6;
  int IO2 = adc_read(muxPB2) / 3;
  int IO3 = adc_read(muxPB3) / 2;

  // Check if any cell voltages are above threshold = 3.5V
  int threshold = 60;  // 17*3.5 ~= 60
  int numBatteryCells = 3;
  if ((IO1 < threshold) | (IO2 - IO1 < threshold) | (IO3 - IO2 < threshold)) {
    return 1;
  }

  return 0;
}
