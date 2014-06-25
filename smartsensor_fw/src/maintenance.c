// This file handles maintenance packets

#include "inc/maintenance.h"

uint8_t compare_ID(uint8_t *buffer);



void maintenancePacket(uint8_t type, uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  switch (type) {
    case 0xFE: maintenancePingPong(data, len, outData, outLen); break;
    default: break;
    // TODO(cduck): Add more maintenance packets
  }
}
void maintenancePingPong(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  if (len < SMART_ID_LEN) return;  // Bad packet
  if (compare_ID(data)) {
    *outLen = len-SMART_ID_LEN;
    uint8_t *pongData = data+SMART_ID_LEN;
    for (int i = len-SMART_ID_LEN-1; i >= 0; --i) {
      outData[i] = pongData[i];
    }
    return;
  }
}



// For ping-pong packet, check if its my ID.
uint8_t compare_ID(uint8_t *buffer) {
  uint8_t val = 1;
  for (uint8_t i = 0; i < SMART_ID_LEN; i++) {
    val &= (smartID[i] == buffer[i]);
  }
  return val;
}
