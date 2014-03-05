#define OP_NAME stack_1
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    ngl_stack_move ( &stack, arg_0.integer );
  }
  #endif

#include <ngl_opcode_end.c>
