#ifndef NGL_ERROR_H_
#define NGL_ERROR_H_

#include <ngl_types_wrapper.h>

extern ngl_error *ngl_ok;
extern ngl_error ngl_cast_error;
extern ngl_error ngl_error_generic;
extern ngl_error ngl_error_internal;
extern ngl_error ngl_error_not_implemented;

ngl_error *ngl_error_init(ngl_error *self, char *msg);

#endif  // NGL_ERROR_H_
