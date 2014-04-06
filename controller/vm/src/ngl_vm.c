#include <ngl_vm.h>
#include <ngl_macros.h>
#include <ngl_alloc.h>
#include <ngl_builtins.h>
#include <ngl_package.h>
#include <ngl_buffer.h>

static ngl_error *
ngl_add_type(ngl_globals * globals, ngl_type * type) {
  ngl_array *array = ngl_alloc_simple(ngl_array, 1);
  if (array == NULL) {
    return &ngl_out_of_memory;
  }
  ngl_ret_on_err(ngl_array_init(array, ngl_type_ngl_obj_ptr));
  ngl_ret_on_err(ngl_table_set(&globals->obj_table,
                               ngl_val_pointer(type), ngl_val_pointer(array)));
  ngl_ret_on_err(ngl_array_push_back(&globals->type_id_to_type_ptr,
                                     ngl_val_pointer(type)));
  return ngl_ok;
}

static ngl_error *
ngl_init_core(ngl_vm * vm) {
  #include <modules/ngl_core.c>

  ngl_globals *globals = &vm->globals;
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_builtin_alien));
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_vm_func));
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_buffer));
  for (ngl_uint i = 0; i < ngl_core_length; i++) {
    ngl_ret_on_err(ngl_globals_set_obj_from_ids(globals, 0, i,
                                                ngl_core[i]));
  }
  return ngl_ok;
}

ngl_error *
ngl_vm_init(ngl_vm * vm) {
  ngl_builtins_init();
  ngl_obj_init(&vm->header, ngl_type_ngl_vm);
  ngl_ret_on_err(ngl_globals_init(&vm->globals));
  ngl_ret_on_err(ngl_array_init(&vm->threads, ngl_type_ngl_thread_ptr));
  ngl_thread *thread = ngl_alloc_simple(ngl_thread, 1);
  ngl_ret_on_err(ngl_thread_init(thread));
  ngl_ret_on_err(ngl_array_push_back(&vm->threads, ngl_val_pointer(thread)));
  ngl_ret_on_err(ngl_init_core(vm));
  return ngl_ok;
}

ngl_define_composite(ngl_vm);
