
#ifndef INC_SMARTSENSOR_ENUMERATION_H_
#define INC_SMARTSENSOR_ENUMERATION_H_

#include "inc/smartsensor/common.h"

extern uint8_t enumerating;

void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len);
void enumerationEnter(uint8_t *data, uint8_t len);
void enumerationExit(uint8_t *data, uint8_t len);
void enumerationReset(uint8_t *data, uint8_t len);
void enumerationSelect(uint8_t *data, uint8_t len);
void enumerationUnselect(uint8_t *data, uint8_t len);

#endif  // INC_SMARTSENSOR_ENUMERATION_H_
