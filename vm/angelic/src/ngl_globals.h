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

#ifndef NGL_GLOBALS_H_
#define NGL_GLOBALS_H_

#include <ngl_types_wrapper.h>

#include <ngl_table_ngl_val.h>
#include <ngl_type.h>

ngl_error *ngl_globals_init(ngl_globals * self);
ngl_error *ngl_globals_init_core(ngl_globals *self);

ngl_error *ngl_globals_get_obj_from_ids(ngl_globals * self,
                                        uint32_t type_id,
                                        uint32_t id, ngl_obj ** res);

ngl_error *ngl_globals_get_obj(ngl_globals * self,
                               ngl_type * type, uint32_t id, ngl_obj ** res);

ngl_error *ngl_globals_set_obj_from_ids(ngl_globals * self,
                                        uint32_t type_id,
                                        uint32_t id, ngl_obj * to_store);

ngl_error *ngl_globals_set_obj(ngl_globals * self,
                               ngl_type * type,
                               uint32_t id, ngl_obj * to_store);

ngl_error *ngl_globals_get_ids(ngl_globals *self,
                               ngl_str *module_name,
                               ngl_str *name,
                               uint32_t *type_id,
                               uint32_t *id);

#endif  // NGL_GLOBALS_H_
