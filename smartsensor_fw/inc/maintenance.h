
#ifndef INC_MAINTENANCE_H_
#define INC_MAINTENANCE_H_

#include "inc/common.h"

void maintenancePacket(uint8_t type, uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen);

void maintenancePingPong(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen);
void maintenanceSlaveConfig(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen);

#endif  // INC_MAINTENANCE_H_
