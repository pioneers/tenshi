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

#ifndef INC_LUFACONFIG_H_
#define INC_LUFACONFIG_H_

#define F_USB F_CPU
#define ARCH ARCH_AVR8

#define USB_DEVICE_ONLY

// Use interrupts for USB
#define INTERRUPT_CONTROL_ENDPOINT

// Default USB_Init settings
#define USE_STATIC_OPTIONS \
  (USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)

#define FIXED_CONTROL_ENDPOINT_SIZE   16
#define FIXED_NUM_CONFIGURATIONS      1

#endif  // INC_LUFACONFIG_H_
