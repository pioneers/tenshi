#include "i2c_register.h"
#include "adc.h"
#include "control_loop.h"
#include "encoder.h"
#include "gitrevision.h"

// This is a temp so that it can be changed all at once (not byte at a time).
static FIXED1616 target_speed_new;
static unsigned char pwm_mode_new;

// Autogenerate if statement needed for provide_i2c_reg.
// offset is the REG_xxx define (i2c register address).
// len is the REG_xxx_LEN define (size of the register).
// val_addr is the address of the variable that should be set.
#define AUTO_PROVIDE_REG(offset,len, var_addr)    \
if (reg >= offset && reg < (offset + len))        \
  return ((unsigned char *)var_addr)[reg-offset];

// Autogenerate if statement needed for set_i2c_reg
// offset is the REG_xxx define (i2c register address)
// len is the REG_xxx_LEN define (size of the register)
// val_addr is the address of the variable that should be set
#define AUTO_SET_REG(offset,len, var_addr)    \
if (reg >= offset && reg < (offset + len))        \
  ((unsigned char *)var_addr)[reg-offset] = val;

extern unsigned char provide_i2c_reg(unsigned char reg) {
  // special
  if (reg == REG_TYPE)
    return REG_TYPE_GRIZZLY_ID;
  AUTO_PROVIDE_REG(REG_PWM_MODE, REG_PWM_MODE_LEN, &pwm_mode_new);
  AUTO_PROVIDE_REG(REG_TARGET_SPEED_NEW, REG_TARGET_SPEED_NEW_LEN,
      &target_speed_new);
  AUTO_PROVIDE_REG(REG_CURRENT_PWM_MODE, REG_CURRENT_PWM_MODE_LEN, &pwm_mode);
  AUTO_PROVIDE_REG(REG_CURRENT_TARGET_SPEED, REG_CURRENT_TARGET_SPEED_LEN,
      get_target_speed_addr());
  AUTO_PROVIDE_REG(REG_ISENSE_ADC, REG_ISENSE_ADC_LEN, get_isense_adc_addr());
  AUTO_PROVIDE_REG(REG_ENCODER_COUNT, REG_ENCODER_COUNT_LEN,
      get_encoder_count_addr());
  AUTO_PROVIDE_REG(REG_REVISION, REG_REVISION_LEN, REVISION);
  AUTO_PROVIDE_REG(REG_ILIMIT_ADC_THRESH, REG_ILIMIT_ADC_THRESH_LEN,
      get_current_limit_adc_threshold_addr());
  AUTO_PROVIDE_REG(REG_ILIMIT_CLAMP_PWM, REG_ILIMIT_CLAMP_PWM_LEN,
      get_current_limit_clamp_pwm_addr());
  AUTO_PROVIDE_REG(REG_ILIMIT_RETRY_TIME, REG_ILIMIT_RETRY_TIME_LEN,
      get_current_limit_retry_time_addr());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_NUM, REG_ILIMIT_RATIO_NUM_LEN,
      get_current_limit_ratio_numerator_addr());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_DENOM, REG_ILIMIT_RATIO_DENOM_LEN,
      get_current_limit_ratio_denominator_addr());
  AUTO_PROVIDE_REG(REG_ILIMIT_RATIO_MAX_USE, REG_ILIMIT_RATIO_MAX_USE_LEN,
      get_current_limit_ratio_max_use_addr());
  AUTO_PROVIDE_REG(REG_ACCEL_LIMIT, REG_ACCEL_LIMIT_LEN,
      get_max_acceleration_addr());

  return UNUSED_REG_VAL;
}

extern void set_i2c_reg(unsigned char reg, unsigned char val) {
  // Special
  if (reg == REG_APPLY_NEW_SPEED) {
    pwm_mode = pwm_mode_new;
    set_target_speed_dangerous(target_speed_new);
  }
  AUTO_SET_REG(REG_PWM_MODE, REG_PWM_MODE_LEN, &pwm_mode_new);
  AUTO_SET_REG(REG_TARGET_SPEED_NEW, REG_TARGET_SPEED_NEW_LEN,
      &target_speed_new);
  AUTO_SET_REG(REG_ILIMIT_ADC_THRESH, REG_ILIMIT_ADC_THRESH_LEN,
      get_current_limit_adc_threshold_addr());
  AUTO_SET_REG(REG_ILIMIT_CLAMP_PWM, REG_ILIMIT_CLAMP_PWM_LEN,
      get_current_limit_clamp_pwm_addr());
  AUTO_SET_REG(REG_ILIMIT_RETRY_TIME, REG_ILIMIT_RETRY_TIME_LEN,
      get_current_limit_retry_time_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_NUM, REG_ILIMIT_RATIO_NUM_LEN,
      get_current_limit_ratio_numerator_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_DENOM, REG_ILIMIT_RATIO_DENOM_LEN,
      get_current_limit_ratio_denominator_addr());
  AUTO_SET_REG(REG_ILIMIT_RATIO_MAX_USE, REG_ILIMIT_RATIO_MAX_USE_LEN,
      get_current_limit_ratio_max_use_addr());
  AUTO_SET_REG(REG_ACCEL_LIMIT, REG_ACCEL_LIMIT_LEN,
      get_max_acceleration_addr());
}
