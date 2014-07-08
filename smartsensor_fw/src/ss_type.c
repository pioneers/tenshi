
#include "inc/smartsensor/ss_type.h"

#include "inc/digital.h"
#include "inc/analog_in.h"

void ssInitType() {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      initDigital();
      break;
    case SENSOR_TYPE_ANALOG_IN:
      initAnalogIn();
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen, uint8_t *inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      activeDigitalSend(decodedBuffer, pacLen,  inband);
      break;
    case SENSOR_TYPE_ANALOG_IN:
      activeAnalogInSend(decodedBuffer, pacLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen, uint8_t inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      activeDigitalRec(decodedBuffer, dataLen, inband);
      break;
    case SENSOR_TYPE_ANALOG_IN:
      activeAnalogInRec(decodedBuffer, dataLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}
