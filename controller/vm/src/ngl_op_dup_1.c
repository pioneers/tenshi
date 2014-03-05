#define OP_NAME dup_1
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  ngl_stack_push ( &stack, ngl_stack_get ( &stack, arg_0.uinteger ) );
  #endif

#include <ngl_opcode_end.c>
