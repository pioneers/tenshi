#define OP_NAME bsrl

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val shift = ngl_stack_pop(&stack);
  ngl_val val = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_uint(val.uinteger >> shift.uinteger));
}
#endif

#include <ngl_opcode_end.c>
