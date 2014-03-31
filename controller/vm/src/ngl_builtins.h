#ifndef NGL_BUILTINS_H_
#define NGL_BUILTINS_H_

#include <ngl_types_wrapper.h>

#include <ngl_macros.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>

ngl_error *ngl_builtins_init();

ngl_declare_base(ngl_uint);
ngl_declare_base(ngl_int);
ngl_declare_base(ngl_float);

ngl_declare_composite(ngl_obj);
ngl_declare_composite(ngl_error);

ngl_declare_alien(ngl_builtin_alien);

ngl_error *ngl_print_float(ngl_float);

#define ngl_call_name ngl_print_float
#define ngl_call_argc 1
#include <ngl_call_declare.h>

#endif  // NGL_BUILTINS_H_
