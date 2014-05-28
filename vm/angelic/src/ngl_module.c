#include <ngl_module.h>
#include <ngl_str.h>
#include <ngl_builtins.h>

ngl_error *ngl_module_init(ngl_module *mod) {
  ngl_ret_on_err(ngl_builtins_init());
  mod->name = ngl_str_lit("unknown");
  ngl_obj_init(&mod->header, ngl_type_ngl_module);
  ngl_ret_on_err(ngl_table_init(&mod->elems, ngl_type_ngl_str,
                                ngl_type_ngl_obj_ptr, &ngl_str_table_i));
  ngl_ret_on_err(ngl_table_init(&mod->elems_ids, ngl_type_ngl_str,
                                ngl_type_ngl_uint, &ngl_str_table_i));
  return ngl_ok;
}


ngl_module *ngl_module_new() {
  ngl_module *mod = ngl_alloc_simple(ngl_module, 1);
  if (mod == NULL) {
    return NULL;
  }
  ngl_error *e = ngl_module_init(mod);
  if (e != ngl_ok) {
    ngl_free(mod);
    return NULL;
  }
  return mod;
}
