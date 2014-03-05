#ifndef NGL_BUILTINS_H
#define NGL_BUILTINS_H
#include <ngl_type_h.h>
#include <ngl_macros.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>

ngl_error * ngl_builtins_init ( );

ngl_declare_base ( ngl_uint );
ngl_declare_base ( ngl_int );
ngl_declare_base ( ngl_float );

ngl_declare_composite ( ngl_obj );
ngl_declare_composite ( ngl_error );

def_struct ( ngl_builtin_alien ) {
  ngl_obj header;
  };

ngl_declare_alien ( ngl_builtin_alien );

ngl_error * ngl_print_float ( ngl_float );

#define ngl_call_name ngl_print_float
#define ngl_call_argc 1
#include <ngl_call_declare.h>

#endif /* end of include guard: NGL_BUILTINS_H */
