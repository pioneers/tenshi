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

#ifndef CONTROL_LOOP_H_
#define CONTROL_LOOP_H_
// This file contains code for the main control loop.
// PID, etc. code is in here.

#include "avr-fixed.h"

#include "util_macro.h"

// Motor controller modes
#define MODE_ENABLE_MASK                0x01

// Speed value interpretation
#define MODE_SPEED_MASK                 0x06
#define MODE_SPEED_RAW                  0x00
#define MODE_SPEED_NO_PID               0x02
#define MODE_SPEED_PID                  0x04

// Sign-magnitude vs. locked-antiphase
#define MODE_SIGN_MAG_LOCKED_ANTIPHASE  0x08
#define MODE_SIGN_MAGNITUDE             0x00
#define MODE_LOCKED_ANTIPHASE           0x08

// Sign-magnitude options
#define MODE_SM_SWITCH_MODE             0x30
#define MODE_SM_GO_COAST                0x00
#define MODE_SM_GO_BRAKE                0x10
#define MODE_SM_CONTROLLED_BRAKE		0x20

#define MODE_SPECIAL_STRESS             0x80

// This is a pre-shifted version of 2047 / 100 rounded down
// This is computed using 2017 / 100 * 65536 where 65536 is the 16.16 scale
// factor.
#define SPEED_TO_PWM_VAL_CONST    1341521

// Current mode setting
// NOTE: one byte; does not need ATOMIC_BLOCK
extern unsigned char pwm_mode;
// Current speed setting. May be interpreted differently depending on mode.
DECLARE_I2C_REGISTER(FIXED1616, target_speed);
// Maximum acceleration, applies to the pwm value (after PID, etc.)
DECLARE_I2C_REGISTER(int, max_acceleration);
// Current limitring registers
DECLARE_I2C_REGISTER(unsigned int, current_limit_adc_threshold);
DECLARE_I2C_REGISTER(int, current_limit_clamp_pwm);
DECLARE_I2C_REGISTER(unsigned int, current_limit_retry_time);
DECLARE_I2C_REGISTER(unsigned char, current_limit_ratio_numerator);
DECLARE_I2C_REGISTER(unsigned char, current_limit_ratio_denominator);
DECLARE_I2C_REGISTER(unsigned int, current_limit_ratio_max_use);

DECLARE_I2C_REGISTER(unsigned long, uptime);

DECLARE_I2C_REGISTER(uint16_t, timeout_period);
DECLARE_I2C_REGISTER(uint16_t, min_switch_delta);

// Number of high-side driver faults
// NOTE: one byte; does not need ATOMIC_BLOCK
extern unsigned char error_count;

// Called to configure control loop internal state on startup.
extern void init_control_loop(void);
// Called to update control loop.
extern void run_control_loop(void);

#endif  // CONTROL_LOOP_H_
