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

// This file has linescan camera related functions

#include <avr/interrupt.h>
#include "inc/linescan.h"
#include "inc/smartsensor_utility.h"
#include "inc/pindef.h"

// Private global variables
// 0: dataBuffer0; 1: dataBuffer1; 2: don't write
static int bufferWriteSelect = 0;
static uint8_t dataBuffer0[8];
static uint8_t dataBuffer1[8];

// Private helper functions

// Public functions called from main.c
void initLinescan() {
  // Does everything to set up the analog stuffs.
  // Turn on pin PC1 (which maps to IN0)
  ADMUX |= (1 << MUX3) | (1 << MUX1);
  #if F_CPU != 8000000
  #error Clock speed not correct
  #endif
  // Enable the ADC and set the division factor between
  // the system clock frequency and the input clock to the ADC.
  // Division factor: 011 = 8
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);

  // Set TOP to 250
  ICR1H = 0x00;
  ICR1L = 0xFA;
  // Following lines ensure CLK is offset from SI
  // Set Compare Match A to 125; CLK
  OCR1AH = 0x00;
  OCR1AL = 0x7D;
  // Set Compare Match B to 0; SI
  OCR1BH = 0x00;
  OCR1BL = 0x00;

  // Set PA6 pin as output; to be used as CLK to linescan camera
  // Set PB3 pin as output; to be used as SI to linescan camera
  DIGITAL_SET_OUT(PWM0);  // PA6
  DIGITAL_SET_OUT(PWM2);  // PB3
  DIGITAL_SET_LOW(PWM0);
  DIGITAL_SET_LOW(PWM2);
  // DIGITAL_SET is also ANALOG_SET apparently...
  DIGITAL_SET_IN(IN1);


  // Set PWM to Fast PWM Mode Operation
  // Clock prescaler = 1/8
  TCCR1A = (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

  // Enable Timer/Counter1 Match A and Match B Interrupts
  TIMSK = (1 << OCIE1A) | (1 << OCIE1B);

  sei();
}

void activeLinescanRec(uint8_t *data, uint8_t len, uint8_t inband) {
}

void activeLinescanSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  *outLen = 8;

  for (int i = 0; i < 8; i++) {
    uint8_t out = 0;
    if (bufferWriteSelect == 1) {
      out = dataBuffer0[i];
    } else if (bufferWriteSelect == 0) {
      out = dataBuffer1[i];
    }
    outData[i] = out;
  }
}

// Interrupt used for manipulating CLK
void interruptTimer1CompareALinescan() {
  static unsigned int dataBufferCount = 0;

  if (dataBufferCount >= 8) {
    dataBufferCount = 0;
    bufferWriteSelect = 1 - bufferWriteSelect;
  }

  DIGITAL_TOGGLE(PWM0);

  uint8_t ref = 0x0F;
  uint8_t muxPB1 = (ref & A_IN1);  // PB1
  int IO1 = adc_read(muxPB1);

  if (bufferWriteSelect != 2) {
    if (bufferWriteSelect == 0) {
      dataBuffer0[dataBufferCount] = IO1 >> 8;
      dataBufferCount++;
      dataBuffer0[dataBufferCount] = IO1;
    } else if (bufferWriteSelect == 1) {
      dataBuffer1[dataBufferCount] = IO1 >> 8;
      dataBufferCount++;
      dataBuffer1[dataBufferCount] = IO1;
    }
  }
}


// Interrupt used for manipulating SI
void interruptTimer1CompareBLinescan() {
  static unsigned int counter = 0;

  if (counter == 1) {
    DIGITAL_SET_HIGH(PWM2);
    bufferWriteSelect = 0;
  }
  if (counter == 2) {
    DIGITAL_SET_LOW(PWM2);
  }
  if (counter == 350) {
    counter = 0;
    bufferWriteSelect = 2;
  }
  counter++;
}

// Private helper functions
