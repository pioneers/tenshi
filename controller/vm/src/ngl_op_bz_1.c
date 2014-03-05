#define OP_NAME bz_1
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    if ( ngl_stack_pop ( &stack ).uinteger == 0 ) {
      pc += arg_0.integer;
      op_bunch = 0;
      }
  }
  #endif

#include <ngl_opcode_end.c>
