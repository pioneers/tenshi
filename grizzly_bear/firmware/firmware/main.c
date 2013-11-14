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

#include "control_loop.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "avr-fixed.h"
#include "LUFA/Drivers/USB/USB.h"

#include "adc.h"
#include "addr_jumper.h"
#include "encoder.h"
#include "led.h"
#include "pindef.h"
#include "pid.h"
#include "pwm.h"
#include "twi_state_machine.h"

// Registers
unsigned char pwm_mode;
DECLARE_I2C_REGISTER_C(FIXED1616, target_speed);

DECLARE_I2C_REGISTER_C(int, max_acceleration);

DECLARE_I2C_REGISTER_C(unsigned int, current_limit_adc_threshold);
DECLARE_I2C_REGISTER_C(int, current_limit_clamp_pwm);
DECLARE_I2C_REGISTER_C(unsigned int, current_limit_retry_time);
DECLARE_I2C_REGISTER_C(unsigned char, current_limit_ratio_numerator);
DECLARE_I2C_REGISTER_C(unsigned char, current_limit_ratio_denominator);
DECLARE_I2C_REGISTER_C(unsigned int, current_limit_ratio_max_use);

DECLARE_I2C_REGISTER_C(unsigned long, uptime);

DECLARE_I2C_REGISTER_C(uint16_t, timeout_period);
DECLARE_I2C_REGISTER_C(uint16_t, min_switch_delta);

unsigned char error_count = 0;

// State variables
static uint16_t current_limit_spike_use = 0;
static unsigned char current_limit_is_clamping;

static unsigned char force_error_led;

static unsigned long last_switch_time = 0;
static unsigned char last_pwm_mode = 0;

// Clamp the input i to be within the interval [a,b]
static inline int clamp_within(int i, int a, int b) {
  if (i < a)
    i = a;
  if (i > b)
    i = b;
  return i;
}

static inline unsigned int u16min(unsigned int a, unsigned int b) {
  if (a < b) {
    return a;
  }
  else {
    return b;
  }
}

static inline unsigned char current_limit_adc_is_overspike(void) {
  unsigned int adc_val = get_isense_adc();
  unsigned int threshold = get_current_limit_adc_threshold();
  // 0x200 is the midpoint of the ADC (2.5v) with represents 0 A
  return adc_val > 0x200 + threshold || adc_val < 0x200 - threshold;
}

static inline void current_limit_check_for_overspike(void) {
  if (current_limit_adc_is_overspike()) {
    // Record an overspike.
    // 0xFF00 is the largest number guaranteed not to overflow when
    // 0xFF is added to it.
    // This prevents an overflow in the NEXT time through.
    unsigned char current_limit_ratio_denominator =
      get_current_limit_ratio_denominator();
    current_limit_spike_use = u16min(0xFF00,
        current_limit_ratio_denominator + current_limit_spike_use);
  }
  else {
    // Record a lack of overspike.
    // This is basically saturating subtraction.
    // It subtracts, but stops at zero.
    unsigned char current_limit_ratio_numerator =
      get_current_limit_ratio_numerator();
    if (current_limit_spike_use <= current_limit_ratio_numerator) {
      current_limit_spike_use = 0;
    }
    else {
      current_limit_spike_use = (current_limit_spike_use -
          current_limit_ratio_numerator);
    }
  }
}

// Current limiting function
// The input is the unsigned value that will be sent to the pwm.
// The direction is controlled by set_pwm_mode and friends, so we
// don't need to deal with it here at all.
static inline int current_limit_clip_speed(int input) {
  static uint16_t powered_down_counter = 0;
  if (powered_down_counter) {
    if (powered_down_counter == get_current_limit_retry_time()) {
      // Try to power up (max value).
      current_limit_is_clamping = 0;
      powered_down_counter = 0;
      force_error_led = 0;
    }
    else {
      // We're powered down, and will stay that way (for now).
      ++powered_down_counter;
    }
  }

  if (current_limit_spike_use > get_current_limit_ratio_max_use()) {
    // Begin ramping down.
    force_error_led = 1;
    current_limit_is_clamping = 1;
    if (!powered_down_counter)
      powered_down_counter = 1;
  }

  if (current_limit_is_clamping) {
    int clamp_pwm = get_current_limit_clamp_pwm();
    input = clamp_within(input, -clamp_pwm, clamp_pwm);
  }

  return input;
}

// Acceleration limiting function
static uint16_t accel_limit_last_speed = 0;

static inline int accel_limit_speed(int speed) {

  int diff = 0;
  int clamped_diff;
  int target_speed;
  int accel_limit = get_max_acceleration();

  diff = speed - accel_limit_last_speed;
  clamped_diff = clamp_within(diff, -accel_limit, accel_limit);
  target_speed = accel_limit_last_speed + clamped_diff;

  accel_limit_last_speed = target_speed;

  return target_speed;
}

void init_control_loop(void) {
  // default values
  // Interrupts aren't enabled here
  set_max_acceleration_dangerous(4);
  set_current_limit_adc_threshold_dangerous(68);
  // 5 Amps
  // Calculated using 5 / ((5 / 1024) * 1000 / 66)
  set_current_limit_clamp_pwm_dangerous(0x200);
  set_current_limit_retry_time_dangerous(1024);
  set_current_limit_ratio_denominator_dangerous(1);
  set_current_limit_ratio_numerator_dangerous(1);
  set_current_limit_ratio_max_use_dangerous(1024);
  set_timeout_period(1024);
  set_min_switch_delta(512);

  // Configure FAULT as input
  DDR(PINDEF_HIGHSIDEFAULT) &= ~(_BV(IO(PINDEF_HIGHSIDEFAULT)));
}

static inline int abs(int in) {
  if (in > 0) {
    return in;
  } else {
    return -in;
  }
}

static inline void update_leds(unsigned char pwm_mode, 
    unsigned char fwd, int pwm_val) {
  if (force_error_led) {
    set_all_leds(LED_ON);
  }
  else if (!(pwm_mode & MODE_ENABLE_MASK)) {
    // Disabled mode
    set_all_leds(LED_OFF);
  }
  else if (abs(pwm_val) < 2) {
    // Turn the LEDs off if the motors are being moved so slowly they are "off"
    set_all_leds(LED_OFF);
  }
  else {
    if (fwd) {
      set_red_led(LED_OFF);
      set_green_led(LED_ON);
    }
    else {
      set_green_led(LED_OFF);
      set_red_led(LED_ON);
    }
  }
}

// configure the PWM into the correct mode (sign-magnitude/locked-antiphase,
// direction, etc)
static inline void set_pwm_mode(unsigned char pwm_mode, unsigned char fwd) {
  if ((pwm_mode & MODE_SIGN_MAG_LOCKED_ANTIPHASE) ==
      MODE_LOCKED_ANTIPHASE) {
    set_locked_antiphase();
  }
  else {
    if ((pwm_mode & MODE_SM_SWITCH_MODE) == MODE_SM_GO_BRAKE) {
      if (fwd)
        set_sign_magnitude_go_brake_fwd();
      else
        set_sign_magnitude_go_brake_bck();
    }
    else if ((pwm_mode & MODE_SM_SWITCH_MODE) == MODE_SM_GO_COAST) {
      if (fwd)
        set_sign_magnitude_go_coast_fwd();
      else
        set_sign_magnitude_go_coast_bck();
    }
    else {
      // Direction doesn't make much sense for controlled brake
      set_controlled_brake();
    }
  }
}

// performs stress mode logic, returns a new pwm_val if stress is enabled
// returns the original pwm_val if stress not enabled
static inline int stress_mode_logic(int pwm_val, unsigned char pwm_mode,
    FIXED1616 target_speed) {
  static unsigned int stress_counter = 0;

  if (pwm_mode & MODE_SPECIAL_STRESS) {
    // hack, these are the fractional bits
    unsigned int stress_period = target_speed & 0xFFFF;
    pwm_val = fixed_to_int(target_speed);
    if (stress_counter < stress_period) {
      pwm_val = -pwm_val;
    }
    stress_counter++;
    if (stress_counter == stress_period * 2)
      stress_counter = 0;
  }
  else {
    // switch out
    stress_counter = 0;
  }

  return pwm_val;
}

// raw mode logic. fwd is always written and is an output parameter. returns
// pwm_val
static inline int raw_mode_logic(FIXED1616 target_speed) {
  int pwm_val;

  pwm_val = fixed_to_int(target_speed);

  return pwm_val;
}

// plain speed mode logic. fwd is always written and is an output parameter.
// returns pwm_val
static inline int no_pid_mode_logic(FIXED1616 target_speed) {
  int pwm_val;

  pwm_val = fixed_to_int(fixed_mult(target_speed, SPEED_TO_PWM_VAL_CONST));

  return pwm_val;
}

static inline void check_timeout() {
  if (get_uptime() > last_i2c_update + get_timeout_period()) {
    pwm_mode = pwm_mode & ~MODE_ENABLE_MASK;
  }
}

static inline void check_fault() {
  // We error on two faults on consecutive cycles. Otherwise, we instantly
  // detect a fault when we switch from disabled to enabled.
  static unsigned char faults = 0;

  //if (!(PIN(PINDEF_HIGHSIDEFAULT) & _BV(IO(PINDEF_HIGHSIDEFAULT)))) {
  if ((PIND & _BV(PD7)) == 0) {
    // The fault pin has been raised
    faults++;
    if (faults >= 2) {
      pwm_mode = pwm_mode & ~MODE_ENABLE_MASK;
      force_error_led = 1;
      // There is no problem with this increment because this is the only writer
      error_count++;
    }
  }
  else {
    faults = 0;
  }
}

void run_control_loop(void) {

  FIXED1616 target_speed_copy;
  unsigned char pwm_mode_copy;

  set_uptime(get_uptime() + 1);
  check_timeout();

  // We need to manually copy these as one block since they go together
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    target_speed_copy = get_target_speed_dangerous();
    pwm_mode_copy = pwm_mode;
  }

  // Prevent too rapid mode switching.
  if (pwm_mode_copy != last_pwm_mode) {
    if ((pwm_mode_copy & MODE_ENABLE_MASK) &&
        get_uptime() < last_switch_time + get_min_switch_delta()) {
      // We haven't reached the switch time delta, so don't switch modes.
      // Unless the mode switch is to disable.
      pwm_mode_copy = last_pwm_mode;
    }
    if (last_pwm_mode != pwm_mode_copy) {
      // If we actually updated the speed, reset the time
      last_switch_time = get_uptime();
      last_pwm_mode = pwm_mode_copy;
    }
  }

  // We always run this code, even when we are disabled. The logic is that
  // if we were overcurrent and we shut off, this code will still run and
  // start decrementing the accumulated overcurrent.
  current_limit_check_for_overspike();

  if (!(pwm_mode_copy & MODE_ENABLE_MASK)) {
    // disabled
    driver_enable(0);
    set_pwm_val(0);
    accel_limit_last_speed = 0;
    update_leds(pwm_mode_copy, 0, 0);
  }
  else {
    // Signed, 12 bits [-0x7ff, 0x7ff]
    int pwm_val = 0;

    // Compute the actual value.
    switch (pwm_mode_copy & MODE_SPEED_MASK) {
      case MODE_SPEED_RAW:
        pwm_val = raw_mode_logic(target_speed_copy);
        break;

      case MODE_SPEED_NO_PID:
        pwm_val = no_pid_mode_logic(target_speed_copy);
        break;

      case MODE_SPEED_PID:
        pwm_val = do_pid_speed(target_speed_copy);
        break;
        
      case MODE_POS_PID:
        pwm_val = do_pid_positional(target_speed_copy);
        break;

      default:
        // Undefined mode, shutting down just in case.
        pwm_mode = 0;
        driver_enable(0);
        set_pwm_val(0);
    }

    // override the value in stress mode
    pwm_val = stress_mode_logic(pwm_val, pwm_mode_copy, target_speed_copy);

    // limit current
    pwm_val = current_limit_clip_speed(pwm_val);
    // limit acceleration
    pwm_val = accel_limit_speed(pwm_val);

    // Compute the mode.
    driver_enable(1);
    set_pwm_mode(pwm_mode_copy, pwm_val >= 0);
    update_leds(pwm_mode_copy, pwm_val >= 0, pwm_val);

    if ((pwm_mode_copy & MODE_SIGN_MAG_LOCKED_ANTIPHASE) ==
        MODE_LOCKED_ANTIPHASE) {
      // Adjustment for locked-antiphase mode
      pwm_val = (pwm_val + 0x800) / 2;
    }
    else {
      // Adjustment for sign-magnitude mode
      pwm_val = pwm_val < 0 ? -pwm_val : pwm_val;
    }
    set_pwm_val(pwm_val);

    // Check for faults. We check here at the end because this is after
    // the driver is enabled. The driver pulls down the fault line when it
    // is held in reset.
    check_fault();
  }
}

void init_hardware(void) {
  unsigned char i2c_addr = determine_addr();
  init_i2c(i2c_addr);
  init_encoder();
  init_adc();
  init_pwm();
  init_control_loop();
  init_led();

  USB_Init();
}

int main(void) {
  init_hardware();

  sei();

  while (1) {
    if (TIFR4 & _BV(TOV4)) {
      TIFR4 = _BV(TOV4);

      // This code is run every time a timer overflow occurs. This currently
      // happens at an frequency of just under 1 kHz (every 1.024 ms).
      read_adc();
      run_control_loop();
    }
  }
}
