/*
 * Tree recursive enumeration of smart sensors
 */

#ifndef INC_SMARTSENSOR_ENUMERATION_H_
#define INC_SMARTSENSOR_ENUMERATION_H_

#include <stdint.h>
#include <string.h>

#include "inc/smartsensor/ssutil.h"


int enumerateSensors(KnownIDs *sensorIDs, uart_serial_module *bus,
  uint8_t busNum);


#endif  // INC_SMARTSENSOR_ENUMERATION_H_
