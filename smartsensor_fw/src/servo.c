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

#include "inc/servo.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t AL1 = 0x01;
static uint8_t AH1 = 0x00;
static uint8_t BL1 = 0x01;
static uint8_t BH1 = 0x00;



// Timer 1 interrupt
ISR(TIMER1_OVF_vect) {

  // Create a counter in order to only send a pulse every 12 timer interrupts.
  static unsigned char counter = 0;

  // On the 12th timer interrupt run this to create a pulse
  if (counter == 0) {

    // Set timer 1 to phase and freq correct pwm mode
    TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1<< COM1B0);
    TCCR1B = (1 << CS10) | (1 << WGM13);

    // update the values in these registers to set the pulse length
    OCR1AH = AH1;
    OCR1AL = AL1;
    OCR1BH = BH1;
    OCR1BL = BL1;

    counter++;

  // Run this after the 12th interrupt to stop the PWM generator from creating a second pulse
  } else if (counter == 1) {

    // Set timer 1 to output disconnected mode
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << CS10) | (1 << WGM12) | (1 << WGM13);
    counter++;
  } else if (counter == 11) {
    // Reset the counter at 11
    counter = 0;
  } else {
    counter++;
  }
}

// Private helper functions

// Public functions called from main.c
void initServo() {
  // Set the PWM pins to output
  DIGITAL_SET_OUT(IN1);
  DIGITAL_SET_OUT(IN3);

  // Initiate timer 1
  TCCR1A = 0;
  TCCR1B = (1 << CS10) | (1 << WGM13);

  // Define the top value of timer 1
  ICR1H = 0x25;
  ICR1L = 0x40;

  // Enable timer 1 overflow interrupt
  TIMSK = (1 << TOIE1);

  // Enable interrupts
  sei();

}

void activeServoRec(uint8_t *data, uint8_t len, uint8_t inband) {
  if (len > 1){
    AL1 = data[0];
    AH1 = data[1];
  }
  if (len > 3){
    BL1 = data[2];
    BH1 = data[3];
  }
}
void activeServoSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  // TODO(cduck): Write this function
  *outlen = 0;
}
