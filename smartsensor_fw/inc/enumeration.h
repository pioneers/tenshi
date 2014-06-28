
#ifndef INC_ENUMERATION_H_
#define INC_ENUMERATION_H_

#include "inc/common.h"

extern uint8_t enumerating;

void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len);
void enumerationEnter(uint8_t *data, uint8_t len);
void enumerationExit(uint8_t *data, uint8_t len);
void enumerationReset(uint8_t *data, uint8_t len);
void enumerationSelect(uint8_t *data, uint8_t len);
void enumerationUnselect(uint8_t *data, uint8_t len);

#endif  // INC_ENUMERATION_H_
