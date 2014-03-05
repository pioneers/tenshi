#include <ngl_obj.h>
#include <ngl_type.h>

void
ngl_obj_init(ngl_obj * obj, ngl_type * type) {
  obj->type = type;
  obj->refc = 1;
}

ngl_uint
ngl_obj_size(ngl_obj * obj) {
  return obj->type->size;
}
