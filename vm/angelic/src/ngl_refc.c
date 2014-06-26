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

#include <ngl_refc.h>

ngl_uint
ngl_refc(ngl_obj * obj) {
  return obj->refc;
}

void
ngl_refd(ngl_obj * obj) {
  if (!obj) {
    return;
  }
  if (obj->refc <= 1) {
    ngl_free(obj);
  } else {
    --obj->refc;
  }
}

void
ngl_refi(ngl_obj * obj) {
  if (!obj) {
    return;
  } else {
    ++obj->refc;
  }
}
