#define OP_NAME write
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    ngl_obj * obj = ngl_stack_pop (&stack).pointer;
    ngl_uint val = ngl_stack_pop (&stack).uinteger;
    ngl_uint * field = (ngl_uint *) obj;
    field[arg_0.uinteger] = val;
  }
  #endif

#include <ngl_opcode_end.c>
