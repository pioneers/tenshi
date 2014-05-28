#ifndef NGL_GLOBALS_H_
#define NGL_GLOBALS_H_

#include <ngl_types_wrapper.h>

#include <ngl_table_ngl_val.h>
#include <ngl_type.h>

ngl_error *ngl_globals_init(ngl_globals * self);
ngl_error *ngl_globals_init_core(ngl_globals *self);

ngl_error *ngl_globals_get_obj_from_ids(ngl_globals * self,
                                        uint32_t type_id,
                                        uint32_t id, ngl_obj ** res);

ngl_error *ngl_globals_get_obj(ngl_globals * self,
                               ngl_type * type, uint32_t id, ngl_obj ** res);

ngl_error *ngl_globals_set_obj_from_ids(ngl_globals * self,
                                        uint32_t type_id,
                                        uint32_t id, ngl_obj * to_store);

ngl_error *ngl_globals_set_obj(ngl_globals * self,
                               ngl_type * type,
                               uint32_t id, ngl_obj * to_store);

ngl_error *ngl_globals_get_ids(ngl_globals *self,
                               ngl_str *module_name,
                               ngl_str *name,
                               uint32_t *type_id,
                               uint32_t *id);

#endif  // NGL_GLOBALS_H_
