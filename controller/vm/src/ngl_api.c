#include <assert.h>
#include <string.h>

#include <ngl_alloc.h>
#include <ngl_api.h>
#include <ngl_builtins.h>
#include <ngl_package.h>
#include <ngl_refc.h>
#include <ngl_table_ngl_val.h>
#include <ngl_vm.h>

ngl_vm *ngl_api_vm_new() {
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

static ngl_error *ngl_module_init(ngl_module *mod) {
  mod->name = ngl_str_lit("unknown");
  ngl_obj_init(&mod->header, ngl_type_ngl_module);
  ngl_ret_on_err(ngl_table_init(&mod->elems, ngl_type_ngl_str,
                                ngl_type_ngl_obj_ptr, &ngl_str_table_i));
  return ngl_ok;
}

static ngl_api_error ngl_api_ret_err(ngl_error *e) {
  char *msg = strdup(e->message->start);
  ngl_refd(&e->header);
  return msg;
}

ngl_module *ngl_api_module_new() {
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

ngl_api_error ngl_api_add_symbol(ngl_module *module, char *name, void *obj) {
  ngl_error *e = ngl_table_set(&module->elems,
                               ngl_val_pointer(ngl_str_new(name, name +
                                   strlen(name))), ngl_val_pointer(obj));
  if (e != ngl_ok) {
    return ngl_api_ret_err(e);
  }
  return NULL;
}

ngl_api_error ngl_api_vm_load_module(ngl_vm *vm, char *name,
                                     ngl_module * mod) {
  /* TODO(kzentner): Remove this restriction. */
  assert(strcmp(name, "core") == 0);
  (void) name;
  ngl_table_iter *iter = ngl_table_iter_new(&mod->elems);
  if (iter == NULL) {
    return "Could not get module elements.";
  }
  while (!ngl_table_iter_done(iter)) {
    ngl_str *name;
    ngl_obj *obj;
    ngl_table_iter_deref(iter, ngl_val_addr(&name), ngl_val_addr(&obj));
    uint32_t type_id;
    uint32_t id;
    ngl_globals_get_ids(&vm->globals, &mod->name, name, &type_id, &id);
    ngl_globals_set_obj_from_ids(&vm->globals, type_id, id, obj);
  }
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
  ngl_thread *main_thread;
  ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  ngl_vm_exec(vm, main_thread, -1);
  return NULL;
}

ngl_api_error ngl_api_vm_run_ticks(ngl_vm *vm, uint32_t ticks) {
  ngl_thread *main_thread;
  ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  ngl_vm_exec(vm, main_thread, ticks);
  return NULL;
}

ngl_api_error ngl_api_start_main(ngl_vm *vm) {
  ngl_thread *main_thread;
  ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread));
  ngl_vm_func *main_func;
  ngl_globals_get_obj_from_ids(&vm->globals, 1, 0, (ngl_obj **) &main_func);
  assert(main_thread->current_func == NULL);
  main_thread->current_func = main_func;
  return NULL;
}
