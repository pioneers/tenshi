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

#include <ngl_type.h>

void
ngl_type_init_header(ngl_type * t) {
  (void) t;
}

void
ngl_type_init_base(ngl_type_base * t, ngl_str name, ngl_uint size) {
  ngl_type_init_header((ngl_type *) t);
  t->size = size;
  t->name = name;
}

ngl_define_composite(ngl_type);
ngl_define_composite(ngl_type_base);
ngl_define_composite(ngl_type_composite);
ngl_define_composite(ngl_type_pointer);

ngl_type *ngl_type_ngl_type_alien;
ngl_type *ngl_type_ngl_type_base;
ngl_type *ngl_type_ngl_type_composite;
ngl_type *ngl_type_ngl_type_pointer;
