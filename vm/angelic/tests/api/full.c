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

#include <assert.h>
#include <ngl_api.h>
#include <stdio.h>

int main(void) {
  ngl_vm * vm = ngl_api_vm_new();
  assert(vm != NULL);
  ngl_module *core = ngl_api_vm_get_module(vm, "core");
  assert(core != NULL);
  void *ex_fn_t = ngl_api_module_get_symbol(core, "ngl_type_ngl_ex_func");
  assert(ex_fn_t != NULL);
  core = ngl_api_vm_get_module(vm, "core");
  assert(core != NULL);
  return 0;
}
