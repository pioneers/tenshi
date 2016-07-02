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

#ifndef NGL_BITS_H_
#define NGL_BITS_H_

#include <stdint.h>
#include <stdbool.h>
#include <ngl_val.h>


extern const void *ngl_end_args;

ngl_int ngl_round_up_power_2(ngl_int to_round);

bool ngl_is_power_2(ngl_int input);

ngl_uint ngl_mask_of_pow2(ngl_uint val);

bool check_mask(ngl_uint mask);


#endif  // NGL_BITS_H_
