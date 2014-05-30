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
extern void set_locked_antiphase(void);
extern void set_controlled_brake(void);

#endif  // PWM_H_
