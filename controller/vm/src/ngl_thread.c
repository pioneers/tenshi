#include <ngl_thread.h>

ngl_error *
ngl_thread_init(ngl_thread * thread) {
  ngl_obj_init(&thread->header, ngl_type_ngl_thread);
  ngl_ret_on_err(ngl_stack_init(&thread->stack));
  ngl_ret_on_err(ngl_stack_init(&thread->call_stack));
  thread->current_func = NULL;
  thread->pc_offset = 0;
  return ngl_ok;
}

ngl_define_composite(ngl_thread);
