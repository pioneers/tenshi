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
void activeAnalogInSend(uint8_t *outData, uint8_t *outLen) {
  *outLen = 1;
  outData[0] = adc_read();  // see below for this one.
}

// Private helper functions

// Taken from http://www.adnbr.co.uk/articles/adc-and-pwm-basics
// by Sumita because I can't code.
// It's for the ATtiny13, though.
int adc_read(void) {
    // I actually use this one. It's the analog in version of DIGITAL_READ.
    // Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait for it to finish
    while (ADCSRA & (1 << ADSC)) {}

    // return ADCH;
    return ADC;  // For 10-bit resolution
}
