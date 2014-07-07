
#ifndef INC_DIGITAL_H_
#define INC_DIGITAL_H_

#include "inc/smartsensor/common.h"

// Public functions called from main.c
void initDigital();
void activeDigitalRec(uint8_t *data, uint8_t len, uint8_t inband);
void activeDigitalSend(uint8_t *outData, uint8_t *outLen);

#endif  // INC_DIGITAL_H_
