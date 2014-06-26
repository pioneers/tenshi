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

#ifndef NGL_TYPE_H_
#define NGL_TYPE_H_

#include <ngl_types_wrapper.h>

#include <ngl_macros.h>
#include <stdlib.h>
#include <ngl_array_ngl_val.h>
#include <ngl_table_ngl_val.h>
#include <ngl_str.h>
#include <ngl_builtins.h>
#include <ngl_hash.h>

#define ngl_type_upcast(ptr_to_subtype) ((ngl_type *)(ptr_to_subtype))

ngl_declare_composite(ngl_type);
ngl_declare_composite(ngl_type_base);
ngl_declare_composite(ngl_type_composite);
ngl_declare_composite(ngl_type_pointer);

void ngl_type_init_header(ngl_type * t);

void ngl_type_init_base(ngl_type_base * t, ngl_str name, ngl_uint size);

extern ngl_type *ngl_type_ngl_type_alien;
extern ngl_type *ngl_type_ngl_type_base;
extern ngl_type *ngl_type_ngl_type_composite;
extern ngl_type *ngl_type_ngl_type_pointer;

#endif  // NGL_TYPE_H_
