#define OP_NAME ret

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  if (ngl_stack_height(&call_stack) == 0) {
    goto exit;
  }

  /* This must be kept in sync with ngl_op_call_1.c */

  ngl_uint stack_offset = ngl_stack_pop(&call_stack).uinteger;
  op_bunch = ngl_stack_pop(&call_stack).uinteger;
  ngl_uint pc_offset = ngl_stack_pop(&call_stack).uinteger;
  func = (ngl_vm_func *) ngl_stack_pop(&call_stack).pointer;
  pc = ngl_vm_func_get_code(func) + pc_offset;
  ngl_val res = ngl_stack_pop(&stack);
  ngl_stack_move(&stack, -stack_offset);
  ngl_stack_push(&stack, res);
}
#endif

#include <ngl_opcode_end.c>
