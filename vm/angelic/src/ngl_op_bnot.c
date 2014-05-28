#define OP_NAME bnot

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val val = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_uint(~val.uinteger));
}
#endif

#include <ngl_opcode_end.c>
