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

#ifndef NGL_STACK_NGL_VAL_H_
#define NGL_STACK_NGL_VAL_H_

#include <ngl_types_wrapper.h>

#include <ngl_val.h>
#include <ngl_error.h>

ngl_error *ngl_stack_init(ngl_stack * self);
ngl_error *ngl_stack_grow(ngl_stack * self, ngl_uint to_copy, ngl_uint size);
ngl_error *ngl_stack_set_size(ngl_stack * self, ngl_uint bucket, ngl_uint size);
ngl_uint ngl_stack_get_size(ngl_stack * self, ngl_uint bucket);
ngl_val ngl_stack_get(ngl_stack * self, ngl_uint idx);
ngl_error *ngl_stack_set(ngl_stack * self, ngl_uint idx, ngl_val val);
ngl_error *ngl_stack_push(ngl_stack * self, ngl_val val);
ngl_val ngl_stack_pop(ngl_stack * self);
ngl_uint ngl_stack_height(ngl_stack * self);
ngl_val *ngl_stack_get_ptr(ngl_stack * self, ngl_uint idx);
void ngl_stack_move(ngl_stack * self, ngl_int diff);

#endif  // NGL_STACK_NGL_VAL_H_
