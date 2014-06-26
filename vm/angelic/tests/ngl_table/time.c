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
#include <string.h>
#include <ngl_call.h>
#include <ngl_str.h>

#define ngl_table_key_t ngl_str_ptr
#define ngl_table_key_kind pointer
#define ngl_table_val_t ngl_str_ptr
#define ngl_table_val_kind pointer
#include <ngl_table.h>

int main() {
  char * test_string = malloc(sizeof("test string") + 1);
  assert(test_string);
  strcpy(test_string, "test string");
  ngl_str * str = ngl_str_new(test_string, NULL);
  ngl_table_ngl_str_ptr_to_ngl_str_ptr table;
  int iterations = 10 * 1000;
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init(&table);
  for(long long i = 0; i < iterations; i++) {
    ngl_table_ngl_str_ptr_to_ngl_str_ptr_set(&table, str, str);
    }
  return 0;
  }
