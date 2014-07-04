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

#include <stdio.h>
#include <string.h>

#include "inc/runtime_entry.h"

int main(int argc, char **argv) {
  printf("Hello world!\n");

  TenshiRuntimeState s = TenshiRuntimeInit();

  printf("Allocated state: %p\n", s);

  const char studentcode[] =
    "x = 42";

  TenshiActorState a;

  int ret = LoadStudentcode(s, studentcode, strlen(studentcode), &a);
  printf("LoadStudentcode: %d, TenshiActorState: %p\n", ret, a);

  TenshiRuntimeDeinit(s);

  printf("Done!\n");

  return 0;
}
