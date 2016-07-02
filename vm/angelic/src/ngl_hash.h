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

#ifndef NGL_HASH_H_
#define NGL_HASH_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>
#include <stdlib.h>             /* size_t */

#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_val.h>

hash_t siphash_24(const ngl_siphash_key key, const uint8_t * msg,
                  const size_t len);

extern ngl_siphash_key ngl_hash_key;

ngl_hash ngl_hash_ngl_val(ngl_val val);
ngl_int ngl_compare_ngl_val(ngl_val a, ngl_val b);

#endif  // NGL_HASH_H_
