#define OP_NAME not

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  ngl_stack_push ( &stack, ngl_val_uint ( ! ngl_stack_pop ( &stack ).uinteger ) );
  #endif

#include <ngl_opcode_end.c>
