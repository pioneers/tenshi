#define OP_NAME iadd

#include <ngl_opcode_begin.c>

#ifndef NGL_NATIVE
#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  ngl_stack_push(&stack, ngl_val_uint(__QADD(left.uinteger, right.uinteger)));
}
#endif
#else
#ifdef OPCODE_BODY
{
  ngl_val right = ngl_stack_pop(&stack);
  ngl_val left = ngl_stack_pop(&stack);
  int64_t sum = (int64_t)(left.integer) + (int64_t)(right.integer);
  if (sum > 2147483647) {
    sum = 2147483647;
  }
  if (sum < -2147483648) {
    sum = -2147483648;
  }
  ngl_stack_push(&stack, ngl_val_int(sum));
}
#endif
#endif

#include <ngl_opcode_end.c>
