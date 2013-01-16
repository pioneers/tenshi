#include "pid.h"
#include "encoder.h"

#define THROTTLE_TO_SPEED_CONST             0x79

DECLARE_I2C_REGISTER_C(FIXED1616, kp);
DECLARE_I2C_REGISTER_C(FIXED1616, ki);
DECLARE_I2C_REGISTER_C(FIXED1616, kd);

// Given a throttle, return a pwm value after running through PID
// Uses enocder feedback only at this point
// This is called from main.c in run_control_loop
// It is not running in interrupt context
int do_pid_loop(FIXED1616 throttle) {
  // PID state
  static int32_t old_encoder_val = 0;
  static int32_t old_speed = 0;
  static FIXED1616 iaccum = 0;

  int32_t encoder_val = get_encoder_count();
  int32_t speed = encoder_val - old_encoder_val;
  old_encoder_val = encoder_val;

  // hack here because it's the same size
  //FIXED1616 target_speed = fixed_mult(throttle, int_to_fixed(THROTTLE_TO_SPEED_CONST));
  int32_t setpoint = 20;//(int32_t)(fixed_to_int(target_speed));

  int32_t error = setpoint - speed;
  //int32_t error = setpoint - encoder_val;

  // pray for no overflow (rest of function too)
  FIXED1616 error_f = int_to_fixed(error);
  // Use derivative on measurement -d(speed)/dt instead of d(error)/dt
  // which equals d(setpoint-speed)/dt which equals -d(speed)/dt if setpoint
  // doesn't change
  FIXED1616 dterm = old_speed - speed;
  FIXED1616 iterm = fixed_mult(int_to_fixed(0), error_f);
  // Use Brett Beuregard's trick of integrating ki*e instead of
  // integrating e and multiplying by ki
  // Also makes it easier to clamp integral windup
  iaccum += iterm;
  // Clamp integral now
  if(iaccum > int_to_fixed(0x7ff))
    iaccum = int_to_fixed(0x7ff);
  else if(iaccum < int_to_fixed(-0x7ff))
    iaccum = int_to_fixed(-0x7ff);

  FIXED1616 outval_f = fixed_mult(int_to_fixed(250), error_f) + fixed_mult(int_to_fixed(0), dterm)
    + iaccum;

  int outval = fixed_to_int(outval_f);
  // Clamp outval
  if(outval > 0x7ff)
    outval = 0x7ff;
  else if(outval < -0x7ff)
    outval = -0x7ff;

  return outval;
}
