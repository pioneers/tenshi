#define OP_NAME fdiv

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_float(left.floating / right.floating));
}
#endif

#include <ngl_opcode_end.c>
