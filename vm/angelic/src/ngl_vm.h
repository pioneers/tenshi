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

#ifndef NGL_VM_H_
#define NGL_VM_H_

#include <ngl_types_wrapper.h>

#include <ngl_func.h>
#include <ngl_error.h>
#include <ngl_macros.h>
#include <ngl_globals.h>
#include <ngl_thread.h>

ngl_declare_composite(ngl_vm);

ngl_error *ngl_vm_exec(ngl_vm * vm, ngl_thread * thread, ngl_int ticks);
ngl_error *ngl_vm_init(ngl_vm * vm);

/* TODO(kzentner): Finish implementing modules and kill this hack. */
extern ngl_uint ngl_vm_core_length;
extern ngl_module_entry * ngl_vm_core;

#endif  // NGL_VM_H_
