#ifndef PWM_H_
#define PWM_H_
// This file contains code for the pwm generation.

// Called to configure PWM hardware on startup.
extern void init_pwm(void);

// Enable or disable high side driver chip.
extern void driver_enable(unsigned char enable);
// Update the PWM timer compare value.
// NOTE: Does not set direction.
extern void set_pwm_val(unsigned int val);

// The following functions set the direction and PWM switching mode.
extern void set_sign_magnitude_go_brake_fwd(void);
extern void set_sign_magnitude_go_brake_bck(void);
extern void set_sign_magnitude_go_coast_fwd(void);
extern void set_sign_magnitude_go_coast_bck(void);
extern void set_locked_antiphase_fwd(void);
extern void set_locked_antiphase_bck(void);

#endif  // PWM_H_
