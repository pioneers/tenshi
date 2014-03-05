#ifndef NGL_ERROR_H_
#define NGL_ERROR_H_
#include <ngl_macros.h>
#include <ngl_type_h.h>

struct ngl_str;

def_struct(ngl_error, {
  ngl_type *type;
  struct ngl_str *message;
})

extern ngl_error *ngl_ok;
extern ngl_error ngl_cast_error;
extern ngl_error ngl_error_generic;
extern ngl_error ngl_error_internal;
extern ngl_error ngl_error_not_implemented;

#endif /* end of include guard: NGL_ERROR_H_ */ /* NOLINT(*) */
