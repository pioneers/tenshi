#include <ngl_func.h>

void ngl_ex_func_init (ngl_func * func, ngl_ex_thunk thunk) {
  ngl_obj_init (&func->ex_func.header, ngl_type_ngl_ex_func);
  func->ex_func.thunk = thunk;
  }

void ngl_vm_func_init (
    ngl_func * func,
    ngl_buffer * code,
    ngl_uint stack_space) {
  ngl_obj_init (&func->ex_func.header, ngl_type_ngl_ex_func);
  func->vm_func.code = code;
  func->vm_func.stack_space = stack_space;
  }

ngl_opbunch * ngl_vm_func_get_code (ngl_vm_func * func) {
  return (ngl_opbunch *) NGL_BUFFER_DATA (func->code);
  }
