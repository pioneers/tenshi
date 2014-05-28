#ifndef NGL_OBJ_H_
#define NGL_OBJ_H_

#include <ngl_types_wrapper.h>

#include <ngl_macros.h>
#include <ngl_val.h>

void ngl_obj_init(ngl_obj * obj, ngl_type * type);

ngl_uint ngl_obj_size(ngl_obj * obj);

#endif  // NGL_OBJ_H_
