#ifndef INC_DRIVER_GLUE_H_
#define INC_DRIVER_GLUE_H_

#include "inc/i2c_master.h"
#include "inc/uart_serial_driver.h"

extern i2c_master_module *i2c1_driver;
extern void i2c1_init(void);

// UART4
extern uart_serial_module *smartsensor_1;
extern void smartsensor1_init(void);

#endif  // INC_DRIVER_GLUE_H_
