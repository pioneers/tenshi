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

#ifndef NGL_CALL_DECLARE_H_
#define NGL_CALL_DECLARE_H_
/* This makes cpplint happy.  ^_^ */
#undef NGL_CALL_DECLARE_H_

/*
 * Used to declare a wrapper around a native function.
 * See ngl_call.h for documentation.
 */

#ifndef ngl_call_name
#error Please define ngl_call_name
#endif

#ifndef ngl_call_argc
#error Please define ngl_call_argc
#endif

#include <ngl_call.h>

/* Declare the associated variables and thunk. */
ngl_error *ngl_func_to_thunk_name(ngl_call_name) (
    ngl_call_interface *inter, ngl_uint argc, ngl_val * arg);
extern int ngl_func_to_thunk_argc(ngl_call_name);
extern ngl_call ngl_func_to_thunk(ngl_call_name);
extern ngl_val
ngl_func_to_thunk_args(ngl_call_name)[ngl_call_argc];
extern ngl_type *ngl_func_to_thunk_types(ngl_call_name)[ngl_call_argc];

#undef ngl_call_name
#undef ngl_call_argc
#endif  // NGL_CALL_DECLARE_H_
