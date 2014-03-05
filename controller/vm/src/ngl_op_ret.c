#define OP_NAME ret

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
  if (ngl_stack_height (&call_stack) == 0) {
    goto exit;
    }
    /* TODO(kzentner): this is completely broken. Fix it. */
    ngl_uint offset = ngl_stack_pop (&call_stack).uinteger;
    func = (ngl_vm_func *) ngl_stack_pop (&call_stack).pointer;
    pc = ngl_vm_func_get_code (func) + offset;
    op_bunch = 0;
  }
  #endif

#include <ngl_opcode_end.c>
