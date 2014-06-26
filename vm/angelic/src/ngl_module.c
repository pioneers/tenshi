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
