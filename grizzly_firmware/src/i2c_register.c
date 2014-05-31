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

#include "inc/i2c_register.h"

#include <avr/pgmspace.h>

#include "inc/adc.h"
#include "inc/control_loop.h"
#include "inc/encoder.h"
#include "inc/pid.h"
#include "inc/version.h"

// This is a temp so that it can be changed all at once (not byte at a time).
static FIXED1616 target_speed_new;
static unsigned char pwm_mode_new;

// Autogenerate if statement needed for provide_i2c_reg.
// offset is the REG_xxx define (i2c register address).
// type is the REG_xxx_TYPE define (type of the register).
// val_val is the value of the variable that should be sent. It may be a
// function, in which case it will only be evaluated when needed.
#define AUTO_PROVIDE_REG(offset, type, var_val)             \
static type __shadow_##offset = 0;                          \
if (reg >= offset && reg < (offset + sizeof(type))) {       \
  if (reg == offset)                                        \
    __shadow_##offset = var_val;                            \
  return ((unsigned char *)&__shadow_##offset)[reg-offset]; \
}

// Autogenerate if statement needed for set_i2c_reg
// offset is the REG_xxx define (i2c register address)
// type is the REG_xxx_TYPE define (type of the register).
// val_addr is the address of the variable that should be set
#define AUTO_SET_REG(offset, type, var_addr)          \
if (reg >= offset && reg < (offset + sizeof(type)))   \
  ((unsigned char *)var_addr)[reg-offset] = val;

extern unsigned char provide_i2c_reg(unsigned char reg) {
  // special
  if (reg == REG_TYPE)
    return REG_TYPE_GRIZZLY_ID;
  AUTO_PROVIDE_REG(REG_PWM_MODE, REG_PWM_MODE_TYPE, pwm_mode_new);
  AUTO_PROVIDE_REG(REG_TARGET_SPEED_NEW, REG_TARGET_SPEED_NEW_TYPE,
      target_speed_new);
  AUTO_PROVIDE_REG(REG_CURRENT_PWM_MODE, REG_CURRENT_PWM_MODE_TYPE, pwm_mode);
  AUTO_PROVIDE_REG(REG_CURRENT_TARGET_SPEED, REG_CURRENT_TARGET_SPEED_TYPE,
      get_target_speed_dangerous());
  AUTO_PROVIDE_REG(REG_ISENSE_ADC, REG_ISENSE_ADC_TYPE,
      get_isense_adc_dangerous());
  AUTO_PROVIDE_REG(REG_ERROR_COUNT, REG_ERROR_COUNT_TYPE, error_count);
  AUTO_PROVIDE_REG(REG_ENCODER_COUNT, REG_ENCODER_COUNT_TYPE,
      get_encoder_count_dangerous());
  AUTO_PROVIDE_REG(REG_PID_KP, REG_PID_KP_TYPE, get_kp_dangerous());
  AUTO_PROVIDE_REG(REG_PID_KI, REG_PID_KI_TYPE, get_ki_dangerous());
  AUTO_PROVIDE_REG(REG_PID_KD, REG_PID_KD_TYPE, get_kd_dangerous());
  // special
  if (reg >= REG_REVISION && reg < (REG_REVISION + REG_REVISION_LEN))
    return pgm_read_byte(&(VERSION_INFORMATION[reg - REG_REVISION]));
  AUTO_PROVIDE_REG(REG_ILIMIT_ADC_THRESH, REG_ILIMIT_ADC_THRESH_TYPE,
      get_current_limit_adc_threshold_dangerous());
  AUTO_PROVIDE_REG(REG_ILIMIT_CLAMP_PWM, REG_ILIMIT_CLAMP_PWM_TYPE,
      get_current_limit_clamp_pwm_dangerous());
  AUTO_PROVIDE_REG(REG_ILIMIT_RETRY_TIME, REG_ILIMIT_RETRY_TIME_TYPE,
      get_current_limit_retry_time_dangerous());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_NUM, REG_ILIMIT_RATIO_NUM_TYPE,
      get_current_limit_ratio_numerator_dangerous());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_DENOM, REG_ILIMIT_RATIO_DENOM_TYPE,
      get_current_limit_ratio_denominator_dangerous());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_MAX_USE, REG_ILIMIT_RATIO_MAX_USE_TYPE,
      get_current_limit_ratio_max_use_dangerous());
  AUTO_PROVIDE_REG(REG_ACCEL_LIMIT, REG_ACCEL_LIMIT_TYPE,
      get_max_acceleration_dangerous());
  AUTO_PROVIDE_REG(REG_UPTIME, REG_UPTIME_TYPE,
      get_uptime_dangerous());
  AUTO_PROVIDE_REG(REG_TIMEOUT_PERIOD, REG_TIMEOUT_PERIOD_TYPE,
      get_timeout_period_dangerous());
  AUTO_PROVIDE_REG(REG_MIN_SWITCH_DELTA, REG_MIN_SWITCH_DELTA_TYPE,
      get_min_switch_delta_dangerous());
  AUTO_PROVIDE_REG(REG_USB_MODE, REG_USB_MODE_TYPE,
      get_usb_mode_dangerous());
  AUTO_PROVIDE_REG(REG_I2C_ADDR, REG_I2C_ADDR_TYPE,
      get_i2c_addr_dangerous());

  return UNUSED_REG_VAL;
}

extern void set_i2c_reg(unsigned char reg, unsigned char val) {
  // Special
  if (reg == REG_APPLY_NEW_SPEED) {
    pwm_mode = pwm_mode_new;
    set_target_speed_dangerous(target_speed_new);
  }
  AUTO_SET_REG(REG_PWM_MODE, REG_PWM_MODE_TYPE, &pwm_mode_new);
  AUTO_SET_REG(REG_TARGET_SPEED_NEW, REG_TARGET_SPEED_NEW_TYPE,
      &target_speed_new);

  AUTO_SET_REG(REG_ENCODER_COUNT, REG_ENCODER_COUNT_TYPE,
      get_encoder_count_addr());

  AUTO_SET_REG(REG_PID_KP, REG_PID_KP_TYPE, get_kp_addr());
  AUTO_SET_REG(REG_PID_KI, REG_PID_KI_TYPE, get_ki_addr());
  AUTO_SET_REG(REG_PID_KD, REG_PID_KD_TYPE, get_kd_addr());
  AUTO_SET_REG(REG_ILIMIT_ADC_THRESH, REG_ILIMIT_ADC_THRESH_TYPE,
      get_current_limit_adc_threshold_addr());
  AUTO_SET_REG(REG_ILIMIT_CLAMP_PWM, REG_ILIMIT_CLAMP_PWM_TYPE,
      get_current_limit_clamp_pwm_addr());
  AUTO_SET_REG(REG_ILIMIT_RETRY_TIME, REG_ILIMIT_RETRY_TIME_TYPE,
      get_current_limit_retry_time_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_NUM, REG_ILIMIT_RATIO_NUM_TYPE,
      get_current_limit_ratio_numerator_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_DENOM, REG_ILIMIT_RATIO_DENOM_TYPE,
      get_current_limit_ratio_denominator_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_MAX_USE, REG_ILIMIT_RATIO_MAX_USE_TYPE,
      get_current_limit_ratio_max_use_addr());
  AUTO_SET_REG(REG_ACCEL_LIMIT, REG_ACCEL_LIMIT_TYPE,
      get_max_acceleration_addr());
  AUTO_SET_REG(REG_TIMEOUT_PERIOD, REG_TIMEOUT_PERIOD_TYPE,
      get_timeout_period_addr());
  AUTO_SET_REG(REG_MIN_SWITCH_DELTA, REG_MIN_SWITCH_DELTA_TYPE,
      get_min_switch_delta_addr());
  AUTO_SET_REG(REG_USB_MODE, REG_USB_MODE_TYPE,
      get_usb_mode_addr());
}
