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

#ifndef NGL_API_H_

#define NGL_API_H_

#include <ngl_types_wrapper.h>

typedef char *ngl_api_error;

ngl_module *ngl_api_module_new(void);

void *ngl_api_module_get_symbol(ngl_module *module, char *name);

ngl_api_error ngl_api_module_set_symbol(ngl_module *module, char *name,
                                        int id, void *obj);

ngl_vm *ngl_api_vm_new(void);

ngl_module *ngl_api_vm_get_module(ngl_vm *vm, char *name);

ngl_api_error ngl_api_vm_set_module(ngl_vm *vm, char *name,
                                    ngl_module * mod);

ngl_api_error ngl_api_vm_load_pkg(ngl_vm *vm, ngl_package *pkg);

ngl_api_error ngl_api_vm_start_main(ngl_vm *vm);

ngl_api_error ngl_api_vm_run(ngl_vm *vm);

ngl_api_error ngl_api_vm_run_ticks(ngl_vm *vm, uint32_t ticks);


#endif  // NGL_API_H_
