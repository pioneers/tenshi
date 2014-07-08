
#ifndef INC_ANALOG_IN_H_
#define INC_ANALOG_IN_H_

#include "inc/smartsensor/common.h"

// Public functions called from main.c
void initAnalogIn();
void activeAnalogInRec(uint8_t *data, uint8_t len, uint8_t inband);
void activeAnalogInSend(uint8_t *outData, uint8_t *outLen, uint8_t inband);
int adc_read(void);

#endif  // INC_ANALOG_IN_H_
