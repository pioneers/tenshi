#define OP_NAME bz_w

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  if (ngl_stack_pop(&stack).uinteger == 0) {
    pc += 1;
    pc += *pc;
    op_bunch = 0;
  }
}
#endif

#include <ngl_opcode_end.c>
