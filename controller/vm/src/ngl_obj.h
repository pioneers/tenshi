#ifndef NGL_OBJ_H_
#define NGL_OBJ_H_
#include <ngl_macros.h>
#include <ngl_type_h.h>
#include <ngl_val.h>

struct ngl_obj {
  ngl_uint refc;
  ngl_type *type;
};

void ngl_obj_init(ngl_obj * obj, ngl_type * type);

ngl_uint ngl_obj_size(ngl_obj * obj);

#endif /* NGL_OBJ_H_ */ /* NOLINT(*) */
