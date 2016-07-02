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

#ifndef INC_AVR_FIXED_H_
#define INC_AVR_FIXED_H_
// This file contains code for fixed point numbers.
// The code is not the most optimal but is less stupid than gcc's output.
// NOTE: Inherited from some other of Robert Ou's projects.
// NOTE: Derived from libfixmath.

#include <stdint.h>

typedef int32_t FIXED1616;

static inline FIXED1616 int_to_fixed(int i) {
  return ((FIXED1616)i) << 16;
}

static inline int fixed_to_int(FIXED1616 i) {
  return i >> 16;
}

extern FIXED1616 fixed_mult(FIXED1616 inArg0, FIXED1616 inArg1);

#endif  // INC_AVR_FIXED_H_
