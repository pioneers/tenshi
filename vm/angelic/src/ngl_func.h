#ifndef NGL_FUNC_H_
#define NGL_FUNC_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>

#include <ngl_error.h>
#include <ngl_obj.h>

ngl_declare_base(ngl_func);
ngl_declare_base(ngl_ex_func);
ngl_declare_base(ngl_vm_func);

void ngl_ex_func_init(ngl_func * func, ngl_ex_thunk * thunk);
void ngl_vm_func_init(ngl_func * func, ngl_buffer * code, ngl_uint stack_space);

ngl_opbunch *ngl_vm_func_get_code(ngl_vm_func * func);

#endif  // NGL_FUNC_H_
