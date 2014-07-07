
#ifndef INC_SMARTSENSOR_SS_TYPE_H_
#define INC_SMARTSENSOR_SS_TYPE_H_

#include <stdint.h>
#include "inc/config.h"

void ssInitType();
void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen);
void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen,
  uint8_t in_band_sigFlag);

#endif  // INC_SMARTSENSOR_SS_TYPE_H_
