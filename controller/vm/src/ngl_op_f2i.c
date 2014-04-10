#define OP_NAME f2i

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val val = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_int(val.floating));
}
#endif

#include <ngl_opcode_end.c>
