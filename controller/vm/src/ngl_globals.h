#ifndef NGL_GLOBAL_H
#define NGL_GLOBAL_H
#include <ngl_type.h>
#include <ngl_table_ngl_val.h>

def_struct ( ngl_globals ) {
  /* type_id -> type */
  ngl_array type_id_to_type_ptr;

  /* type -> [obj] */
  ngl_table obj_table;
  };

ngl_error * ngl_globals_init ( ngl_globals * self );

ngl_error * ngl_globals_get_obj_from_ids (
    ngl_globals * self,
    uint32_t type_id,
    uint32_t id,
    ngl_obj ** res
    );

ngl_error * ngl_globals_get_obj (
    ngl_globals * self,
    ngl_type * type,
    uint32_t id,
    ngl_obj ** res
    );

ngl_error * ngl_globals_set_obj_from_ids (
    ngl_globals * self,
    uint32_t type_id,
    uint32_t id,
    ngl_obj * to_store
    );

ngl_error * ngl_globals_set_obj (
    ngl_globals * self,
    ngl_type * type,
    uint32_t id,
    ngl_obj * to_store
    );

#endif /* end of include guard: NGL_GLOBAL_H */
