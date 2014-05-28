#define OP_NAME bsra

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val shift = ngl_stack_pop(&stack);
  ngl_val val = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_int(val.integer >> shift.uinteger));
}
#endif

#include <ngl_opcode_end.c>
