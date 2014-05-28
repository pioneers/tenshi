#include <assert.h>
#include <string.h>

#include <ngl_alloc.h>
#include <ngl_api.h>
#include <ngl_builtins.h>
#include <ngl_package.h>
#include <ngl_refc.h>
#include <ngl_table_ngl_val.h>
#include <ngl_vm.h>
#include <ngl_module.h>

static ngl_api_error ngl_api_ret_err(ngl_error *e) {
  char *msg = "An unknown error occurred.";
  if (e->message != NULL && e->message->start != NULL) {
    msg = strdup(e->message->start);
    }
  return msg;
}

ngl_module *ngl_api_module_new(void) {
  return ngl_module_new();
}

void *ngl_api_module_get_symbol(ngl_module *module, char *name) {
  ngl_val out;
  ngl_str key = ngl_str_from_dynamic(name);
  ngl_error *e = ngl_table_get(&module->elems,
                               ngl_val_pointer(&key),
                               ngl_val_addr(&out));
  if (e != ngl_ok) {
    return NULL;
  }
  return out.pointer;
}

ngl_api_error ngl_api_module_set_symbol(ngl_module *module, char *name,
                                        int id, void *obj) {
  /* We need to duplicate the name because the caller may re-use the memory. */
  ngl_str * name_s = ngl_str_new(strdup(name), NULL);
  ngl_error *e = ngl_table_set(&module->elems,
                               ngl_val_pointer(name_s),
                               ngl_val_pointer(obj));
  if (e != ngl_ok) {
    ngl_free(name);
    return ngl_api_ret_err(e);
  }
  e = ngl_table_set(&module->elems_ids,
                    ngl_val_pointer(name_s),
                    ngl_val_uint(id));
  if (e != ngl_ok) {
    ngl_free(name);
    return ngl_api_ret_err(e);
  }
  /*
   * TODO(kzentner): Use reference counting to prevent leaking name if the key
   * already exists in the table.
   */
  return NULL;
}

ngl_vm *ngl_api_vm_new(void) {
  ngl_vm *vm = ngl_alloc_simple(ngl_vm, 1);
  if (vm == NULL) {
    return NULL;
  }
  ngl_error *e = ngl_vm_init(vm);
  if (e != ngl_ok) {
    ngl_free(vm);
    return NULL;
  }
  return vm;
}

ngl_module *ngl_api_vm_get_module(ngl_vm *vm, char *name) {
  ngl_str name_s = ngl_str_from_dynamic(name);
  ngl_module *res = NULL;
  ngl_str_print(name_s);
  ngl_table_get(&vm->globals.module_table,
                ngl_val_pointer(&name_s),
                ngl_val_addr(&res));
  return res;
}

ngl_api_error ngl_api_vm_set_module(ngl_vm *vm, char *name,
                                    ngl_module * mod) {
  /* TODO(kzentner): Remove this restriction. */
  assert(strcmp(name, "core") == 0);
  /*
   * Currently, this function copies from the "core" ngl_module object to
   * globals with type_id 0.
   * This seems kind of hacky, and should probably be changed.
   */

  /* Iterate through each element in the module. */
  ngl_str * mod_name = ngl_str_new(name, NULL);
  ngl_table_iter *iter = ngl_table_iter_new(&mod->elems);
  if (iter == NULL) {
    ngl_free(mod_name);
    return "Could not get module elements.";
  }
  while (!ngl_table_iter_done(iter)) {
    ngl_str *obj_name;
    ngl_obj *obj;
    ngl_table_iter_deref(iter, ngl_val_addr(&obj_name), ngl_val_addr(&obj));
    uint32_t type_id = 0;
    uint32_t id = 0;
    ngl_error * e = ngl_globals_get_ids(&vm->globals, mod_name,
                                        obj_name, &type_id, &id);
    if (e != ngl_ok) {
      /*
       * If we couldn't get the id by looking in the module, look in elem_ids.
       * This implies that the ngl_core.yaml file is more authoritative than
       * whoever called ngl_api_module_set_symbol.
       * Quite likely, this should be the other way around, but the id provided
       * by that function doesn't fully work. See that function for details.
       */
      ngl_uint id_get = 0;
      ngl_table_get(&mod->elems_ids,
                    ngl_val_pointer(obj_name),
                    ngl_val_addr(&id_get));
      id = id_get;
    }
    ngl_globals_set_obj_from_ids(&vm->globals, type_id, id, obj);
    ngl_table_iter_next(iter);
  }
  ngl_free(mod_name);
  ngl_free(iter);
  return NULL;
}

ngl_api_error ngl_api_vm_load_pkg(ngl_vm *vm, ngl_package *pkg) {
  ngl_error *e = ngl_package_apply(pkg, vm);
  if (e != ngl_ok) {
    return ngl_api_ret_err(e);
  }
  return NULL;
}

ngl_api_error ngl_api_vm_run(ngl_vm *vm) {
  ngl_error * e = ngl_ok;
  ngl_thread *main_thread;
  e = ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  /*
   * Note that -1 indicates an infinite number of ticks. This is not exposed in
   * this api though.
   */
  e = ngl_vm_exec(vm, main_thread, -1);
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  return NULL;
}

ngl_api_error ngl_api_vm_run_ticks(ngl_vm *vm, uint32_t ticks) {
  ngl_thread *main_thread = NULL;
  ngl_error * e = ngl_ok;
  e = ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  e = ngl_vm_exec(vm, main_thread, ticks);
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  return NULL;
}

ngl_api_error ngl_api_vm_start_main(ngl_vm *vm) {
  ngl_error * e = ngl_ok;
  ngl_thread *main_thread = NULL;
  e = ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  ngl_vm_func *main_func = NULL;
  e = ngl_globals_get_obj_from_ids(&vm->globals, 1, 0, (ngl_obj **) &main_func);
  if (e != ngl_ok) {
    ngl_api_ret_err(e);
  }
  assert(main_thread->current_func == NULL);
  main_thread->current_func = main_func;
  return NULL;
}
