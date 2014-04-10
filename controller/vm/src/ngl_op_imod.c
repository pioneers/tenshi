#define OP_NAME imod

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_int(left.integer % right.integer));
}
#endif

#include <ngl_opcode_end.c>
