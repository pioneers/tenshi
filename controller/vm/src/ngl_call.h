#ifndef NGL_CALL_H_
#define NGL_CALL_H_
#include <stdlib.h>

#include <ngl_macros.h>
#include <ngl_type_h.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>
#include <ngl_func.h>

#define ngl_arg(type, kind, value) \
ngl_type_to_ngl_type(type), ngl_check_to_ngl_val_##kind(type, value)

def_struct(ngl_call, {
  ngl_obj header;
  ngl_val *args;
  ngl_func func;
  ngl_uint num_args;
  ngl_uint filled_args;
  ngl_type **arg_types;
})

ngl_declare_composite(ngl_call);

ngl_error *ngl_call_push(ngl_call * self, ngl_type * type, ngl_val val);
ngl_error *ngl_call_force_push(ngl_call * self, ngl_val val);
ngl_error *ngl_call_pop(ngl_call * self, ngl_uint count);

ngl_error *ngl_call_push_types(ngl_call * self, ngl_uint count, ...);

ngl_error *ngl_invoke(ngl_call * self);

ngl_error *ngl_call_clone(ngl_call * dest, ngl_call * src);
ngl_error *ngl_call_deinit(ngl_call * c);
ngl_error *ngl_call_delete(ngl_call * c);
void ngl_init_null_call(ngl_call * call);

extern ngl_call ngl_null_call;

#define ngl_func_to_thunk_name(func_name) \
(join_token(ngl_call_thunk_, func_name))

#define ngl_func_to_thunk(func_name) \
(join_token(ngl_call_, func_name))

#define ngl_func_to_thunk_argc(func_name) \
(join_token(ngl_call_argc_, func_name))

#define ngl_func_to_thunk_args(func_name) \
(join_token(ngl_call_args_, func_name))

#define ngl_func_to_thunk_types(func_name) \
(join_token(ngl_call_types_, func_name))


#endif /* end of include guard: NGL_CALL_H_ */ /* NOLINT(*) */
