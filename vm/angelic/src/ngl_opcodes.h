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

#ifndef NGL_OPCODES_H_
#define NGL_OPCODES_H_
#include <stdint.h>
#include <math.h>
#include <ngl_macros.h>
#include <assert.h>
#include <limits.h>
#include <ngl_stack_ngl_val.h>
#ifdef NGL_ARM
#include <inc/stm32f4xx.h>
#endif
#ifdef NGL_EMCC
#include <emscripten.h>
#endif

def_union(ngl_asm_arg, {
  int8_t integer;
  uint8_t uinteger;
})

typedef uint8_t ngl_asm_op;

#define OP_MAX_ARGS 3

#endif  // NGL_OPCODES_H_
