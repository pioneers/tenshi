#define OP_NAME i2f

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val val = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_float(val.integer));
}
#endif

#include <ngl_opcode_end.c>
