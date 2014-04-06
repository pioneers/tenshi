#define OP_NAME call_1
#define OP_NUM_ARGS 1

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  ngl_uint arg_count = arg_0.uinteger;
  ngl_obj *f = ngl_stack_get(&stack, arg_count).pointer;
  ngl_error *error = ngl_ok;
  if (f->type == ngl_type_ngl_vm_func) {
    /* This must be kept in sync with ngl_op_ret.c */

    /* Store the current function. */
    ngl_stack_push(&call_stack, ngl_val_pointer(func));
    /* Store the offset into the current function. */
    ngl_stack_push(&call_stack, ngl_val_uint(pc - ngl_vm_func_get_code(func)));
    /* Remove the current arguments from the op_bunch and save it. */
    ngl_stack_push(&call_stack, ngl_val_uint(op_bunch >> 8 * OP_NUM_ARGS));
    /* Save the stack offset for the return. */
    ngl_stack_push(&call_stack, ngl_val_uint(arg_count + 1));

    op_bunch = 0;
    func = (ngl_vm_func *) f;
    pc = ngl_vm_func_get_code(func);
  } else {
    ngl_ex_thunk * thunk = ((ngl_ex_func *) f)->thunk;

    /*
     * -1 is because index arg_count contains the function, not the
     *  arguments.
     */
    error = thunk(NULL, arg_count,
                  ngl_stack_get_ptr(&stack, arg_count - 1));

    ngl_val res = ngl_stack_pop(&stack);
    ngl_stack_move(&stack, -arg_count);
    if (error == ngl_ok) {
      ngl_stack_push(&stack, res);
    } else {
      ngl_stack_push(&stack, ngl_val_pointer(error));
      /* TODO(kzentner): Implement exceptions. */
      /* goto target_throw; */
    }
  }
}
#endif

#include <ngl_opcode_end.c>
