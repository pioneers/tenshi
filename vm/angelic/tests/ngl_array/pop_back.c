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
#include <stdio.h>
#include <ngl_error.h>
#include <ngl_builtins.h>

#define ngl_array_elem_t ngl_int
#define ngl_array_elem_kind integer
#include <ngl_array.h>

int main() {
  ngl_builtins_init();
  ngl_error * e = ngl_ok;
  ngl_array_ngl_int a;
  ngl_err(e, ngl_array_ngl_int_init(&a));
  ngl_array_ngl_int_iter iter = ngl_array_ngl_int_start(&a);
  ngl_int i = 100;
  ngl_err(e, ngl_array_ngl_int_push_back(&a, i));
  ngl_int i2 = 0;
  ngl_err(e, ngl_array_ngl_int_pop_back(&a, &i2));
  printf("i2 = %" ngl_int_format "\n", i2);
  assert(i2 == 100);
  assert(! ngl_array_ngl_int_good(&iter));
  assert(ngl_array_ngl_int_length(&a) == 0);
  return e != ngl_ok;
  }
