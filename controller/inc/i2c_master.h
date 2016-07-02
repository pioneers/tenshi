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

#ifndef INC_I2C_MASTER_H_
#define INC_I2C_MASTER_H_

#include <stdint.h>
#include <stdlib.h>

// TODO(rqou): Define some kind of common sensor interface

typedef struct tag_i2c_master_module {
  // No stuff here yet
} i2c_master_module;

// The only possible transactions are send only, receive only, or send then
// receive.
#define I2C_TRANSACTION_STATUS_QUEUED     1
#define I2C_TRANSACTION_STATUS_SENDING    2
#define I2C_TRANSACTION_STATUS_RECEIVING  3
#define I2C_TRANSACTION_STATUS_DONE       4
#define I2C_TRANSACTION_STATUS_ERROR      5

extern i2c_master_module *i2c_master_init_module(void *periph_base);
extern void *i2c_issue_transaction(i2c_master_module *module, uint8_t addr,
  uint8_t *data_out, size_t len_out, uint8_t *data_in, size_t len_in);
extern int i2c_transaction_status(i2c_master_module *module,
  void *transaction);
extern void i2c_handle_interrupt(i2c_master_module *module);
extern void i2c_handle_interrupt_error(i2c_master_module *module);
extern int i2c_transaction_finish(i2c_master_module *module,
  void *transaction);

#endif  // INC_I2C_MASTER_H_
