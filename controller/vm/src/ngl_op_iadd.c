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
  if (sum > INT_MAX) {
    sum = INT_MAX;
  }
  if (sum < INT_MIN) {
    sum = INT_MIN;
  }
  ngl_stack_push(&stack, ngl_val_int(sum));
}
#endif
#endif

#include <ngl_opcode_end.c>
