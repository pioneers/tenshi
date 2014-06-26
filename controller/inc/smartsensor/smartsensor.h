#ifndef INC_SMARTSENSOR_SMARTSENSOR_H_
#define INC_SMARTSENSOR_SMARTSENSOR_H_

#include "inc/FreeRTOS.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/uart_serial_driver.h"

#define SS_NUM_SAMPLES 8  // 3 bits of resolution
#define SS_NUM_FRAMES 6
#define SS_FIRST_FRAME 1  // 1 indexed frame numbers


void smartsensor_init();


portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters);
portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters);

#endif  // INC_SMARTSENSOR_SMARTSENSOR_H_
