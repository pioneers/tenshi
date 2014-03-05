#define OP_NAME set_1
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  ngl_stack_set ( &stack, arg_0.uinteger, ngl_stack_get ( &stack, 0 ) );
  ngl_stack_pop ( &stack );
  #endif

#include <ngl_opcode_end.c>
