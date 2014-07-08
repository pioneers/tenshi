
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

void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen, uint8_t *inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_GRIZZLY3:
      activeGrizzly3Send(decodedBuffer, pacLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen, uint8_t inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_GRIZZLY3:
      activeGrizzly3Rec(decodedBuffer, dataLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}
