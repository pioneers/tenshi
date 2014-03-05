#define OP_NAME read
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_obj *obj = ngl_stack_pop(&stack).pointer;
  ngl_uint *field = (ngl_uint *) obj;
  ngl_stack_push(&stack, ngl_val_uint(field[arg_0.integer]));
}
#endif

#include <ngl_opcode_end.c>
