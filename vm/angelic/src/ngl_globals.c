#include <ngl_globals.h>
#include <ngl_macros.h>
#include <assert.h>
#include <ngl_vm.h>
#include <string.h>
#include <ngl_module.h>

ngl_error *
ngl_globals_init(ngl_globals * self) {
  ngl_ret_on_err(ngl_array_init(&self->type_id_to_type_ptr,
                                ngl_type_ngl_type_ptr));
  ngl_ret_on_err(ngl_table_init(&self->obj_table,
                                ngl_type_ngl_type_ptr,
                                ngl_type_ngl_obj_ptr,
                                &ngl_table_ngl_val_interface));
  ngl_ret_on_err(ngl_table_init(&self->module_table,
                                ngl_type_ngl_str_ptr,
                                ngl_type_ngl_module_ptr,
                                &ngl_str_table_i));
  return ngl_ok;
}

ngl_error *
ngl_globals_init_core(ngl_globals *self) {
  /* 
   * This function creates an ngl_module object containing the objects from the
   * ngl_vm_core global variable. It then stores that module into the module
   * table as the "core" module.
   * Note that modifying the resulting core module does not change what
   * packages will be linked against, unless ngl_api_vm_set_module is used.
   */

  /* TODO(kzentner): Refactor modules. */
  /*
   * Currently, module related code is in ngl_api, ngl_globals, ngl_vm, and
   * ngl_module.
   * This should be consolidated.
   */

  /* Create the core module. */
  ngl_module *core = ngl_module_new();
  if (core == NULL) {
    return &ngl_out_of_memory;
  }

  /* Fill the core with objects from objects in ngl_vm_core. */
  for (uint32_t i = 0; i < ngl_vm_core_length; i++) {
    ngl_str * name = ngl_str_new(ngl_vm_core[i].name, NULL);
    ngl_ret_on_err(ngl_table_set(&core->elems, ngl_val_pointer(name),
                                 ngl_val_pointer(ngl_vm_core[i].cobj)));
  }

  /* Set the "core" module to be the newly created module. */
  ngl_str * core_name = ngl_str_new("core", NULL);
  ngl_ret_on_err(ngl_table_set(&self->module_table,
                               ngl_val_pointer(core_name),
                               ngl_val_pointer(core)));
  /*
   * TODO(kzentner): Free objects on error conditions.
   * This is somewhat complicated because not all the corresponding deinit
   * functions exist.
   */

  return ngl_ok;
}

ngl_error *
ngl_globals_get_obj_from_ids(ngl_globals * self,
                             uint32_t type_id, uint32_t id, ngl_obj ** res) {
  ngl_type *type;
  ngl_error *e = ngl_array_get(&self->type_id_to_type_ptr,
                               type_id,
                               ngl_val_addr(&type));
  if (e != ngl_ok) {
    return e;
  }
  e = ngl_globals_get_obj(self, type, id, res);
  return e;
}

ngl_error *
ngl_globals_get_obj(ngl_globals * self,
                    ngl_type * type, uint32_t id, ngl_obj ** res) {
  ngl_array *array;
  ngl_error *e = ngl_table_get(&self->obj_table,
                               ngl_val_pointer(type),
                               ngl_val_addr(&array));
  if (e != ngl_ok) {
    return e;
  }
  for (ngl_int i = 0; i < ngl_array_length(array); i++) {
    void *p;
    ngl_array_get(array, i, ngl_val_addr(&p));
  }
  return ngl_array_get(array, id, ngl_val_addr(res));
}

ngl_error *
ngl_globals_set_obj_from_ids(ngl_globals * self,
                             uint32_t type_id,
                             uint32_t id, ngl_obj * to_store) {
  ngl_type *type;
  ngl_error *e = ngl_array_get(&self->type_id_to_type_ptr,
                               type_id,
                               ngl_val_addr(&type));
  if (e != ngl_ok) {
    return e;
  }
  return ngl_globals_set_obj(self, type, id, to_store);
}

ngl_error *
ngl_globals_set_obj(ngl_globals * self,
                    ngl_type * type, uint32_t id, ngl_obj * to_store) {
  ngl_array *array;
  ngl_error *e = ngl_table_get(&self->obj_table,
                               ngl_val_pointer(type),
                               ngl_val_addr(&array));
  if (e != ngl_ok) {
    return e;
  }
  for (ngl_uint i = ngl_array_length(array); i <= id; i++) {
    ngl_array_push_back(array, ngl_val_pointer(NULL));
  }
  return ngl_array_set(array, id, ngl_val_pointer(to_store));
}

ngl_error *ngl_globals_get_ids(ngl_globals *self,
                               ngl_str *module_name,
                               ngl_str *name,
                               uint32_t *type_id,
                               uint32_t *id) {
  (void) self;
  /* TODO(kzentner): Remove this limitation. */
  assert(strcmp(module_name->start, "core") == 0);
  (void) module_name;

  /*
   * Do a linear scan of ngl_vm_core to find the index / id. Note that this
   * completely ignores the elem_ids field of the "core" ngl_module object.
   * TODO(kzentner): Fix this.
   */
  for (uint32_t i = 0; i < ngl_vm_core_length; i++) {
    if (strcmp(ngl_vm_core[i].name, name->start) == 0) {
      *type_id = 0;
      *id = i;
      return ngl_ok;
    }
  }
  return &ngl_error_generic;
}
