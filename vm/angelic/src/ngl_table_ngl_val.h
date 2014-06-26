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

#ifndef NGL_TABLE_NGL_VAL_H_
#define NGL_TABLE_NGL_VAL_H_

#include <ngl_types_wrapper.h>

#include <stdbool.h>
#include <stdlib.h>

#include <ngl_call.h>
#include <ngl_error.h>
#include <ngl_hash.h>
#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_val.h>

ngl_table *ngl_table_new(ngl_type * key_type,
                         ngl_type * val_type,
                         ngl_table_elem_interface * hash_interface);

ngl_error *ngl_table_init_to_size(ngl_table * self,
                                  ngl_type * key_type,
                                  ngl_type * val_type,
                                  size_t predicted_elems,
                                  ngl_table_elem_interface * hash_interface);

ngl_error *ngl_table_init(ngl_table * self,
                          ngl_type * key_type,
                          ngl_type * val_type,
                          ngl_table_elem_interface * hash_interface);

ngl_error *ngl_table_get(ngl_table * self, ngl_val key, ngl_val * dst);
ngl_error *ngl_table_get_hash(ngl_table * self, ngl_val key, ngl_val * dst,
                              ngl_hash hash);


ngl_error *ngl_table_get_or_add(ngl_table * self,
                                ngl_val key,
                                ngl_val * dst, ngl_call * on_missing);

ngl_error *ngl_table_get_or_add_hash(ngl_table * self,
                                     ngl_val key,
                                     ngl_val * dst,
                                     ngl_call * on_missing, ngl_hash hash);

ngl_error *ngl_table_get_default(ngl_table * self,
                                 ngl_val key,
                                 ngl_val * dst, ngl_val default_value);
ngl_error *ngl_table_get_default_hash(ngl_table * self,
                                      ngl_val key,
                                      ngl_val * dst,
                                      ngl_val default_value, ngl_hash hash);

ngl_error *ngl_table_set(ngl_table * self, ngl_val key, ngl_val value);
ngl_error *ngl_table_set_hash(ngl_table * self,
                              ngl_val key, ngl_val value, ngl_hash hash);

ngl_error *ngl_table_delete(ngl_table * self);

ngl_error *ngl_table_iter_init(ngl_table_iter *self, ngl_table *parent);
ngl_table_iter *ngl_table_iter_new(ngl_table *parent);

ngl_error *ngl_table_iter_next(ngl_table_iter *self);
bool ngl_table_iter_done(ngl_table_iter *self);
ngl_error *ngl_table_iter_deref(ngl_table_iter *self, ngl_val *key,
                                ngl_val *val);

extern ngl_table_elem_interface ngl_table_ngl_val_interface;
extern ngl_error ngl_table_not_found;
extern ngl_error ngl_iter_done;

#endif  // NGL_TABLE_NGL_VAL_H_
