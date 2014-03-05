#define OP_NAME fmod

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_float(fmod(left.floating, right.floating)));
}
#endif

#include <ngl_opcode_end.c>
