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

#define OP_NAME ret

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  if (ngl_stack_height(&call_stack) == 0) {
    goto exit;
  }

  /* This must be kept in sync with ngl_op_call_1.c */

  ngl_uint stack_offset = ngl_stack_pop(&call_stack).uinteger;
  op_bunch = ngl_stack_pop(&call_stack).uinteger;
  ngl_uint pc_offset = ngl_stack_pop(&call_stack).uinteger;
  func = (ngl_vm_func *) ngl_stack_pop(&call_stack).pointer;
  pc = ngl_vm_func_get_code(func) + pc_offset;
  ngl_val res = ngl_stack_pop(&stack);
  ngl_stack_move(&stack, -stack_offset);
  ngl_stack_push(&stack, res);
}
#endif

#include <ngl_opcode_end.c>
