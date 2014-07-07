
// This file lets the smart sensor code call grizzly code

#include "inc/smartsensor/ss_type.h"

#include "inc/ss_grizzly3.h"

void ssInitType() {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_GRIZZLY3:
      initGrizzly3();
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_GRIZZLY3:
      activeGrizzly3Send(decodedBuffer, pacLen);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen,
  uint8_t in_band_sigFlag) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_GRIZZLY3:
      activeGrizzly3Rec(decodedBuffer, dataLen, in_band_sigFlag);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}
