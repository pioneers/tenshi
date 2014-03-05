#include <ngl_vm.h>
#include <stdio.h>
#include <stdbool.h>
#include <ngl_opcodes.h>
#include <ngl_stack_ngl_val.h>

#if !defined (HAVE_COMPUTED_GOTOS) && defined (USE_COMPUTED_GOTOS)
  #error "Cannot use computed goto with this compiler."
  #endif

#ifdef USE_COMPUTED_GOTOS
  #define DISPATCH goto *jump_targets[op_bunch & 0xff];
#else
  #define DISPATCH switch (op_bunch & 0xff)
  #endif

ngl_error * ngl_vm_exec (
    ngl_vm * vm,
    ngl_thread * restrict thread,
    ngl_vm_func * func
) {
  (void) vm;
  ngl_opbunch * pc = ngl_vm_func_get_code (func);
  ngl_opbunch op_bunch = *pc++;
#define args ((ngl_asm_arg *) &op_bunch)
  ngl_stack stack = thread->stack;
  ngl_stack call_stack = thread->call_stack;

  #define arg_0 (args[0])
  #define arg_1 (args[1])
  #define arg_2 (args[2])

  #ifdef USE_COMPUTED_GOTOS
    static const void * jump_targets[] = {
      #define OPCODE_TARGET
      #include <ngl_opcodes.c>
      };
    #endif

  while (true) {
    DISPATCH {
      #define OPCODE_LABEL
      #define OPCODE_BODY
      #include <ngl_opcodes.c>
      }
    }
  exit:
  thread->stack = stack;
  thread->call_stack = call_stack;
  return ngl_ok;
  }
