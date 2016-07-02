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

#ifndef NGL_VAL_H_
#define NGL_VAL_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>
#include <inttypes.h>

#include <ngl_macros.h>

ngl_declare_base(ngl_val);

#define ngl_val_int(val) ((ngl_val) {.integer = (ngl_int)(val)})
#define ngl_val_uint(val) ((ngl_val) {.uinteger = (ngl_uint)(val)})
#define ngl_val_float(val) ((ngl_val) {.floating = (ngl_float)(val)})
#define ngl_val_pointer(val) ((ngl_val) {.pointer = (ngl_obj *)(val)})
#define ngl_val_addr(val_addr) ((ngl_val *) val_addr)

#endif  // NGL_VAL_H_
