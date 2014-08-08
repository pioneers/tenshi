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

#ifndef ngl_call_name
#error Please define ngl_call_name
#endif

/*
 * Used to define a wrapper around a native function.
 * See ngl_call.h for documentation.
 */

#ifndef ngl_call_args
#error Please define ngl_call_args
#endif

#ifndef ngl_call_argc
#error Please define ngl_call_argc
#endif

#ifndef ngl_call_return
#define ngl_call_return(x) (x)
#endif

#ifndef ngl_call_pre
#define ngl_call_pre()
#endif

#ifndef ngl_call_post
#define ngl_call_post()
#endif

#include <ngl_call.h>

#define NGL_ARG_INT(idx, type) ((type) arg[(idx)].integer)
#define NGL_ARG_UINT(idx, type) ((type) arg[(idx)].uinteger)
#define NGL_ARG_FLOAT(idx, type) ((type) arg[(idx)].floating)
#define NGL_ARG_PTR(idx, type) ((type) arg[(idx)].pointer)

#define NGL_RET(type) (NGL_RET_IDX(0, type))
#define NGL_RET_IDX(idx, type) ((type *) &arg[(idx) + (ngl_call_argc)])

#define NGL_ERROR() (&error)

#define NGL_ARG_INTERFACE() (interface)

#define NGL_ARGC() (argc)

/* Define the associated variables and thunk. */

ngl_error * ngl_func_to_thunk_name(ngl_call_name) (
    ngl_call_interface *interface, ngl_uint argc, ngl_val * arg) {
  ngl_error * error = ngl_ok;
  /*
   * ngl_call_args expands to multiple arguments of the form
   * NGL_ARG_INT(0, int), NGL_ARG_FLOAT(1, double), etc. This causes the C
   * compiler to output appropriate conversion code.
   */
  (void) interface;
  (void) argc;
  (void) arg;
  ngl_call_pre();
  ngl_call_return(ngl_call_name(ngl_call_args));
  ngl_call_post();
  return error;
}

int ngl_func_to_thunk_argc(ngl_call_name) = ngl_call_argc;
ngl_call ngl_func_to_thunk(ngl_call_name);
ngl_val
ngl_func_to_thunk_args(ngl_call_name)[ngl_call_argc];
ngl_type *ngl_func_to_thunk_types(ngl_call_name)[ngl_call_argc];

#undef ngl_call_name
#undef ngl_call_args
#undef ngl_call_argc
#undef NGL_ARGC
#undef NGL_ARG_INTERFACE
#undef NGL_RET
#undef NGL_ERROR
#undef NGL_ARG_PTR
#undef ngl_call_return
#undef ngl_call_pre
#undef NGL_ARG_UINT
#undef ngl_call_post
#undef NGL_ARG_INT
#undef NGL_RET_IDX
#undef NGL_ARG_FLOAT

