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

#define OP_NAME isub

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
#ifdef NGL_ARM
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_uint(__QSUB(left.uinteger, right.uinteger)));
}
#else
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  int64_t sum = (int64_t)(left.integer) - (int64_t)(right.integer);
  if (sum > INT_MAX) {
    sum = INT_MAX;
  }
  if (sum < INT_MIN) {
    sum = INT_MIN;
  }
  ngl_stack_push(&stack, ngl_val_int(sum));
}
#endif
#endif

#include <ngl_opcode_end.c>
