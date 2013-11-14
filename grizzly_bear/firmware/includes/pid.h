#ifndef PID_H_
#define PID_H_

#include "avr-fixed.h"
#include "util_macro.h"

// PID constants

DECLARE_I2C_REGISTER(FIXED1616, kp);
DECLARE_I2C_REGISTER(FIXED1616, ki);
DECLARE_I2C_REGISTER(FIXED1616, kd);

// Given a target_speed, return a pwm value after running through PID
// Uses enocder feedback only at this point
extern int do_pid_speed(FIXED1616 target_speed);
extern int do_pid_positional(FIXED1616 target_speed);

#endif
