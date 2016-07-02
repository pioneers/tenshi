// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#include <ngl_vm.h>
#include <ngl_macros.h>
#include <ngl_alloc.h>
#include <ngl_builtins.h>
#include <ngl_package.h>
#include <ngl_buffer.h>
#include <string.h>
#include <ngl_func.h>

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

ngl_module_entry * ngl_vm_core;
ngl_uint ngl_vm_core_length;


static ngl_error *
ngl_init_core(ngl_vm * vm) {
  /* TODO(kzentner): Refactor modules. */
  /*
   * Currently, module related code is in ngl_api, ngl_globals, ngl_vm, and
   * ngl_module.
   * This should be consolidated.
   */

  /* Include the generated core file. */
  #include <modules/ngl_core.c>


  /* Copy the core array to a global variable. */
  ngl_vm_core = ngl_alloc_simple(ngl_module_entry,
                                 ngl_core_length);
  if (ngl_vm_core == NULL) {
    return &ngl_out_of_memory;
  }
  memcpy((void *) ngl_vm_core, (void *) ngl_core,
         sizeof(ngl_module_entry) * ngl_core_length);
  ngl_vm_core_length = ngl_core_length;

  /* Add some type ids. Note that the order here determines the id numbers. */
  ngl_globals *globals = &vm->globals;
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_builtin_alien));
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_vm_func));
  ngl_ret_on_err(ngl_add_type(globals, ngl_type_ngl_buffer));

  /* Load the objects from the core into the globals. */
  for (ngl_uint i = 0; i < ngl_core_length; i++) {
    ngl_ret_on_err(ngl_globals_set_obj_from_ids(globals, 0, i,
                                                ngl_core[i].cobj));
  }
  return ngl_ok;
}

ngl_error *
ngl_vm_init(ngl_vm * vm) {
  ngl_builtins_init();
  ngl_obj_init(&vm->header, ngl_type_ngl_vm);
  ngl_ret_on_err(ngl_globals_init(&vm->globals));
  ngl_ret_on_err(ngl_init_core(vm));
  ngl_ret_on_err(ngl_globals_init_core(&vm->globals));

  ngl_ret_on_err(ngl_array_init(&vm->threads, ngl_type_ngl_thread_ptr));
  ngl_thread *thread = ngl_alloc_simple(ngl_thread, 1);
  ngl_ret_on_err(ngl_thread_init(thread));
  ngl_ret_on_err(ngl_array_push_back(&vm->threads, ngl_val_pointer(thread)));
  return ngl_ok;
}

ngl_define_composite(ngl_vm);
