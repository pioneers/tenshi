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

#ifndef LED_H_
#define LED_H_

// Initializes the led's.
void init_led();

// Turn the green led on or off.
void set_green_led(unsigned char on);

// Turn the red led on or off.
void set_red_led(unsigned char on);

// Turn the red and green led's on or off.
// The amber led cannot be controlled by software.
void set_all_leds(unsigned char on);

#define LED_ON 1
#define LED_OFF 0


#endif /* LED_H_ */
