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
  } else if (f->type == ngl_type_ngl_ex_func) {
    ngl_ex_thunk * thunk = ((ngl_ex_func *) f)->thunk;

    /*
     * -1 is because index arg_count contains the function, not the
     *  arguments.
     */

    ngl_val * fn_args = ngl_stack_get_ptr(&stack, arg_count - 1);

    /* Push a spot onto the stack for the return value of the function. */
    ngl_stack_push(&stack, ngl_val_uint(0));

#ifdef NGL_EMCC
    /*
     * If we're on emscripten, retrieve the actual javascript function,
     * Then apply it to an array ripped out of the stack.
     */
    EM_ASM_INT({
        /*
         * Function "pointers" referring to external functions are indices in
         * an array, which are desnsely packed in functionPointers, starting at
         * index 0.  However, the function pointer values are 2, 4, 6, 8, etc.
         * This implies the following mapping.
         */
        var func_num = ($0 / 2) - 1;
        var the_function = Module.Runtime.functionPointers[func_num];
        var stack = Module.HEAPF32.buffer.slice($1 >> 2, ($1 + $2) >> 2);
        var args = [];
        for (var i = 0; i < $2; i++) {
          /*
           * TODO(kzentner): This assumes all arguments are floats.
           * Unfortunately, fixing the involves changing the api, since there
           * is no way of passing the argument types here.
           */
          args.push(Module.HEAPF32[i + ($1 >> 2)]);
          }
        /* Save the return value (must be a float for the time being). */
        Module.HEAPF32[$1 >> 2] = the_function.apply(null, args);
      }, thunk, fn_args, arg_count);
#else
    error = thunk(NULL, arg_count, fn_args);
#endif

    ngl_val res = ngl_stack_pop(&stack);

    /* The -1 is to remove the arguments *and* the function from the stack. */
    ngl_stack_move(&stack, -arg_count - 1);
    if (error == ngl_ok) {
      ngl_stack_push(&stack, res);
    } else {
      ngl_stack_push(&stack, ngl_val_pointer(error));
      /* TODO(kzentner): Implement exceptions. */
      /* goto target_throw; */
    }
  } else {
    assert(0);
  }
}
#endif

#include <ngl_opcode_end.c>
