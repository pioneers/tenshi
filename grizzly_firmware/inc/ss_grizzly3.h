
#ifndef INC_SS_GRIZZLY3_H_
#define INC_SS_GRIZZLY3_H_

#include "inc/smartsensor/common.h"

void initGrizzly3();
void activeGrizzly3Rec(uint8_t *data, uint8_t len, uint8_t inband);
void activeGrizzly3Send(uint8_t *outData, uint8_t *outLen, uint8_t *inband);

#endif  // INC_SS_GRIZZLY3_H_
