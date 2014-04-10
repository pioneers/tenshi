#define OP_NAME imul

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  int64_t product = (int64_t)(left.integer) * (int64_t)(right.integer);
  if (product > 2147483647) {
    product = 2147483647;
  }
  if (product < -2147483648) {
    product = -2147483648;
  }
  ngl_stack_push(&stack, ngl_val_int(product));
}
#endif

#include <ngl_opcode_end.c>
