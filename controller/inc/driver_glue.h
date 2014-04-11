#ifndef INC_DRIVER_GLUE_H_
#define INC_DRIVER_GLUE_H_

#include "inc/debug_uart.h"
#include "inc/i2c_master.h"
#include "inc/uart_serial_driver.h"

extern i2c_master_module *i2c1_driver;
extern void i2c1_init(void);

// UART4
extern uart_serial_module *smartsensor_1;
extern void smartsensor1_init(void);

// UART2
extern uart_serial_module *smartsensor_2;
extern void smartsensor2_init(void);

// UART1
extern uart_serial_module *smartsensor_3;
extern void smartsensor3_init(void);

// UART6
extern uart_serial_module *smartsensor_4;
extern void smartsensor4_init(void);

// UART3
extern uart_serial_module *radio_driver;
extern void radio_driver_init(void);

// Debug SPI/UART
extern debug_uart_module *debug_uart;
extern void global_debug_uart_init(void);

#endif  // INC_DRIVER_GLUE_H_
