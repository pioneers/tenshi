// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#ifndef INC_DRIVER_GLUE_H_
#define INC_DRIVER_GLUE_H_

#include "inc/i2c_master.h"
#include "inc/uart_serial_driver.h"

#define SMART_SENSOR_BAUD 500000

extern i2c_master_module *i2c1_driver;
extern void i2c1_init(void);


extern uart_serial_module *ssBusses[4];

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

#endif  // INC_DRIVER_GLUE_H_
