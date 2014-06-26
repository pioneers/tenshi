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

#include <ngl_bits.h>
#include <stdio.h>
#include <assert.h>

const void *ngl_end_args = (void *) ~(intptr_t) 0;

/* Unaligned pointer value at end of memory space(probably in kernel space).
 * Used to indicate the last argument in a variable argument list of pointers
 * where NULL would indicate allocation failure.
 */

ngl_int
ngl_round_up_power_2(ngl_int to_round) {
  ngl_int next = 0;
  while ((next = (to_round & (to_round - 1)))) {
    to_round = next;
  }
  return to_round * 2;
}

bool
ngl_is_power_2(ngl_int input) {
  return !(input & (input - 1));
}

ngl_uint
ngl_mask_of_pow2(ngl_uint val) {
  ngl_uint result = 1;
  while (val > 2) {
    result = (result << 1) | result;
    val = val / 2;
  }
  return result;
}

bool
check_mask(ngl_uint mask) {
  ngl_uint d = mask;
  while (d > 0) {
    if ((d & mask) != d) {
      return false;
      }
    d >>= 1;
  }
  return true;
}
