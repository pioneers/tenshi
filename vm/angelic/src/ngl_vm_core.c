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

#include <ngl_vm.h>
#include <stdio.h>
#include <stdbool.h>
#include <ngl_opcodes.h>

#if !defined(HAVE_COMPUTED_GOTOS) && defined(USE_COMPUTED_GOTOS)
#error "Cannot use computed goto with this compiler."
#endif

#ifdef USE_COMPUTED_GOTOS
#define DISPATCH goto *jump_targets[op_bunch & 0xff];
#else
#define DISPATCH switch(op_bunch & 0xff) /* NOLINT(*) */
#endif

ngl_error *
ngl_vm_exec(ngl_vm * vm, ngl_thread * restrict thread, ngl_int ticks) {
  (void) vm;
  ngl_int ticks_remaining = ticks;
  ngl_vm_func * func = thread->current_func;
  if (func == NULL) {
    return &ngl_error_generic;
  }
  ngl_opbunch *pc = ngl_vm_func_get_code(func);
  pc += thread->pc_offset;
  ngl_opbunch op_bunch = *pc++;
#define args ((ngl_asm_arg *) &op_bunch)
  ngl_stack stack = thread->stack;
  ngl_stack call_stack = thread->call_stack;

#define arg_0 (args[0])
#define arg_1 (args[1])
#define arg_2 (args[2])

#ifdef USE_COMPUTED_GOTOS
  static const void *jump_targets[] = ({
#define OPCODE_TARGET
#include <ngl_opcodes.c>
  });
#endif

  while (true) {
    DISPATCH {
#define OPCODE_LABEL
#define OPCODE_BODY
#include <ngl_opcodes.c> /* NOLINT(*) */
    }
  }
exit:
  thread->stack = stack;
  thread->call_stack = call_stack;
  thread->current_func = func;
  return ngl_ok;
}
