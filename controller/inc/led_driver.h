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

#ifndef INC_LED_DRIVER_H_
#define INC_LED_DRIVER_H_

#include <stdint.h>

// Configures all the LEDs and turns them all on. Very basic, so should never
// crash. Used to indicate hard errors like failed oscillator init.
extern void led_driver_panic(void);

// Initializes the LED driver. Defaults to pattern 0. Patterns are defined in
// the .c file.
extern void led_driver_init(void);

// Default "chaser" sequence: 1 sec of each LED
#define PATTERN_DEFAULT_CHASER    0
// LEDs go back and forth quickly
#define PATTERN_BACK_AND_FORTH    1
// Just red flashes
#define PATTERN_JUST_RED    2
// Flashes pairs of LEDs during sensor enumeration
#define PATTERN_ENUMERATING    3

// Set the LED pattern
extern void led_driver_set_mode(uint8_t mode);

// Get the current LED pattern
extern uint8_t led_driver_get_mode(void);

// Set some LEDs to states in pattern and not flash
extern void led_driver_set_fixed(uint8_t pattern, uint8_t mask);
extern uint8_t led_driver_get_fixed_pattern();
extern uint8_t led_driver_get_fixed_mask();

#endif  // INC_LED_DRIVER_H_
