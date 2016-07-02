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
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>
#ifdef NGL_EMCC
#include <emscripten.h>
#endif

int
main(int argc, char *argv[]) {
#ifdef NGL_EMCC
  EM_ASM(
      FS.mkdir('/real');
      FS.mount(NODEFS, { root : '/' }, '/real'););
#endif
  if (argc != 2) {
    printf("%s: Usage '%s <package>'\n", argv[0], argv[0]);
    return 1;
  }
  ngl_buffer *program = ngl_buffer_from_filename(ngl_str_from_static(argv[1]));
  if (program == NULL) {
    printf("%s: Could not load package '%s'\n", argv[0], argv[1]);
    return 1;
  }
  ngl_error *e = ngl_run_package((ngl_package *) NGL_BUFFER_DATA(program));
  if (e != ngl_ok) {
    printf("%s: Encountered error running package.\n", argv[0]);
  }
  return 0;
}
