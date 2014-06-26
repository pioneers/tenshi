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

#include <ngl_error.h>
#include <stdio.h>
#include <ngl_builtins.h>
#include <ngl_str.h>
#include <ngl_alloc.h>

ngl_error ngl_cast_error;

ngl_error *ngl_ok = 0;
ngl_error ngl_error_generic;
ngl_error ngl_error_internal;
ngl_error ngl_error_not_implemented;

ngl_error *ngl_error_init(ngl_error *self, char *msg) {
  ngl_obj_init(&self->header, ngl_type_ngl_error);
  self->message = ngl_str_new(msg, NULL);
  if (self->message == NULL) {
    return &ngl_out_of_memory;
  }
  return ngl_ok;
}
