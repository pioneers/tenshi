#include <ngl_type.h>

void
ngl_type_init_header(ngl_type * t) {
  (void) t;
}

void
ngl_type_init_base(ngl_type_base * t, ngl_str name, ngl_uint size) {
  ngl_type_init_header((ngl_type *) t);
  t->size = size;
  t->name = name;
}

ngl_define_composite(ngl_type);
ngl_define_composite(ngl_type_base);
ngl_define_composite(ngl_type_composite);
ngl_define_composite(ngl_type_pointer);

ngl_type *ngl_type_ngl_type_alien;
ngl_type *ngl_type_ngl_type_base;
ngl_type *ngl_type_ngl_type_composite;
ngl_type *ngl_type_ngl_type_pointer;
