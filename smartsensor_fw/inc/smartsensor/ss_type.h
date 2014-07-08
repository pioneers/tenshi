
#ifndef INC_SMARTSENSOR_SS_TYPE_H_
#define INC_SMARTSENSOR_SS_TYPE_H_

#include <stdint.h>
#include "inc/config.h"

void ssInitType();
void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen, uint8_t *inband);
void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen, uint8_t inband);

#endif  // INC_SMARTSENSOR_SS_TYPE_H_
