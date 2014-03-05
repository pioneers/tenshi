#ifndef NGL_FUNC_H_
#define NGL_FUNC_H_
#include <stdint.h>
#include <ngl_type_h.h>
#include <ngl_obj.h>
#include <ngl_error.h>
#include <ngl_buffer_h.h>

typedef uint32_t ngl_opbunch;

def_struct(ngl_vm_func, {
  ngl_obj header;
  ngl_buffer *code;
  ngl_uint stack_space;
})

typedef ngl_error *(*ngl_ex_thunk) (ngl_val * args);

def_struct(ngl_ex_func, {
  ngl_obj header;
  ngl_ex_thunk thunk;
})

def_union(ngl_func, {
  ngl_obj header;
  ngl_vm_func vm_func;
  ngl_ex_func ex_func;
})

ngl_declare_base(ngl_func);
ngl_declare_base(ngl_ex_func);
ngl_declare_base(ngl_vm_func);

void ngl_ex_func_init(ngl_func * func, ngl_ex_thunk thunk);
void ngl_vm_func_init(ngl_func * func, ngl_buffer * code, ngl_uint stack_space);

ngl_opbunch *ngl_vm_func_get_code(ngl_vm_func * func);

#endif /* end of include guard: NGL_FUNC_H_ */ /* NOLINT(*) */
