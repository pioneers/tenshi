#ifndef INC_SMARTSENSOR_SMARTSENSOR_H_
#define INC_SMARTSENSOR_SMARTSENSOR_H_

#include "inc/smartsensor/ssutil.h"

#include "inc/FreeRTOS.h"

#include "inc/uart_serial_driver.h"


portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters);
portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters);

#endif  // INC_SMARTSENSOR_SMARTSENSOR_H_
