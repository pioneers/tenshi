#include <ngl_error.h>
#include <stdio.h>
#include <ngl_builtins.h>
#include <ngl_str.h>
#include <ngl_alloc.h>

ngl_error ngl_cast_error;

ngl_error *ngl_ok = 0;
ngl_error ngl_error_generic;
ngl_error ngl_error_internal;
ngl_error ngl_error_not_implemented;

ngl_error *ngl_error_init(ngl_error *self, char *msg) {
  ngl_obj_init(&self->header, ngl_type_ngl_error);
  self->message = ngl_str_new(msg, NULL);
  if (self->message == NULL) {
    return &ngl_out_of_memory;
  }
  return ngl_ok;
}
