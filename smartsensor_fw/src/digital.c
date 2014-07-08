// This file has digital io smart sensor related functions

#include "inc/digital.h"

// Private global variables
// None

// Private helper functions
uint8_t make_flags(uint8_t canIn, uint8_t pushPull, uint8_t openDr,
  uint8_t outAL);


// Public functions called from main.c
void initDigital() {
  DIGITAL_PULLUP_ON(IN0);
  DIGITAL_SET_HIGH(IN1);
  DIGITAL_SET_LOW(IN2);
  DIGITAL_SET_LOW(IN3);

  // TODO(tobinsarah): allow configuration as input/output for each relevant pin
  DIGITAL_SET_IN(IN0);
  DIGITAL_SET_OUT(IN1);
  DIGITAL_SET_OUT(IN2);
  DIGITAL_SET_OUT(IN3);
}
void activeDigitalRec(uint8_t *data, uint8_t len, uint8_t inband) {
  if (len >= 1) {
    DIGITAL_SET(IN0, data[0] & 1);  // UPDATE IN0
    DIGITAL_SET(IN1, data[0] & 2);  // UPDATE IN1
    DIGITAL_SET(IN2, data[0] & 4);  // UPDATE IN2
    DIGITAL_SET(IN3, data[0] & 8);  // UPDATE IN3
  }
}
void activeDigitalSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  *outLen = 1;
  outData[0] = (~(DIGITAL_READ(IN0))) & 0x1;
}





// Private helper functions

// assemble flags byte for DIO
//   bit0 of flags -- can input
//   bit1 of flags -- input is active low
//   bit2 of flags -- can output push-pull
//   bit3 of flags -- can output open-drain
//   bit4 of flags -- output is active low
uint8_t make_flags(uint8_t canIn, uint8_t pushPull, uint8_t openDr,
  uint8_t outAL) {
  uint8_t flagbyte = (uint8_t) (canIn & 1);
  flagbyte |= ((DIGITAL_READ(IN0)) << 1);
  flagbyte |= ((pushPull & 1) << 2);
  flagbyte |= ((openDr & 1) << 3);
  flagbyte |= ((outAL & 1) << 4);
  return flagbyte;
}
