#define OP_NAME imul

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  int64_t product = (int64_t)(left.integer) * (int64_t)(right.integer);
  if (product > INT_MAX) {
    product = INT_MAX;
  }
  if (product < INT_MIN) {
    product = INT_MIN;
  }
  ngl_stack_push(&stack, ngl_val_int(product));
}
#endif

#include <ngl_opcode_end.c>
