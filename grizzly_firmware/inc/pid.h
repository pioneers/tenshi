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

#ifndef INC_PID_H_
#define INC_PID_H_

#include "inc/avr-fixed.h"
#include "inc/util_macro.h"

// PID constants

DECLARE_I2C_REGISTER(FIXED1616, kp);
DECLARE_I2C_REGISTER(FIXED1616, ki);
DECLARE_I2C_REGISTER(FIXED1616, kd);

// Given a target_speed, return a pwm value after running through PID
// Uses enocder feedback only at this point
extern int do_pid_speed(FIXED1616 target_speed);
extern int do_pid_positional(FIXED1616 target_speed);

#endif  // INC_PID_H_
