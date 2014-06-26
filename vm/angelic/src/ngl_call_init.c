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
 * Used to initialize a wrapper around a native function.
 * See ngl_call.h for documentation.
 */

ngl_ex_func_init(&ngl_func_to_thunk(ngl_call_name).func,
                 &ngl_func_to_thunk_name(ngl_call_name));
ngl_func_to_thunk(ngl_call_name).args = ngl_func_to_thunk_args(ngl_call_name);
ngl_func_to_thunk(ngl_call_name).arg_types =
ngl_func_to_thunk_types(ngl_call_name);
ngl_func_to_thunk(ngl_call_name).num_args =
ngl_func_to_thunk_argc(ngl_call_name);
ngl_func_to_thunk(ngl_call_name).filled_args = 0;
#ifdef ngl_call_ngl_types
ngl_call_push_types(&ngl_func_to_thunk(ngl_call_name),
                    ngl_func_to_thunk_argc(ngl_call_name), ngl_call_ngl_types);
#undef ngl_call_ngl_types
#endif

#undef ngl_call_name
#undef ngl_call_types
