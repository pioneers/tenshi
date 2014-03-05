#define OP_NAME eq

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    ngl_stack_push ( &stack, ngl_val_uint ( ngl_stack_pop ( &stack ).integer == ngl_stack_pop ( &stack ).integer ) );
  }
  #endif

#include <ngl_opcode_end.c>
