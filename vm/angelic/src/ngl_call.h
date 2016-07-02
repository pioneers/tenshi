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

#ifndef NGL_CALL_H_
#define NGL_CALL_H_

#include <ngl_types_wrapper.h>

#include <stdlib.h>

#include <ngl_macros.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>
#include <ngl_func.h>

/*
 * An ngl_call is basically a delegate. It holds a reference to a (possibly
 * native) function, and array of types that the arguments to this function
 * should match, and an (possibly incomplete) array of arguments.
 *
 * There is also an api for (relatively) easily wrapping external functions
 * with ngl_call's so that they can be used from inside the vm.
 */

/*
 * This function defines the external api for *using* ngl_call's.
 * Unfortunately, defining an ngl_call to wrap a function is most cleanly done
 * using multiple file macros.  To avoid fragmenting the documentation for how
 * to do so, the documentation for defining an ngl_call to wrap a function is
 * also defined here.
 */

/*
 * Here is an example of how to wrap a function:
 *
 * in a header:
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_argc 2
 * #include <ngl_call_declare.h>
 *
 * somewhere else in a source file:
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_args NGL_ARG_INT(0, int), NGL_ARG_PTR(1, char *)
 * #define ngl_call_argc 2
 * #include <ngl_call_define.c>
 *
 * somewhere else in a function which is called before the wrapper is used:
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_ngl_types ngl_type_int, ngl_type_char_ptr
 * #include <ngl_call_init.c>
 *
 */



#define ngl_arg(type, kind, value) \
ngl_type_to_ngl_type(type), ngl_check_to_ngl_val_##kind(type, value)

ngl_declare_composite(ngl_call);

/* Push a type and argument to the ngl_call. */
ngl_error *ngl_call_push(ngl_call * self, ngl_type * type, ngl_val val);

/* Push a type to the ngl_call, ignoring the recorded type. */
ngl_error *ngl_call_force_push(ngl_call * self, ngl_val val);

/* Remove the <count> last objects from the ngl_call. */
ngl_error *ngl_call_pop(ngl_call * self, ngl_uint count);

/* Add <count> new types to the ngl_call. */
ngl_error *ngl_call_push_types(ngl_call * self, ngl_uint count, ...);

/* Invoke the ngl_call. */
ngl_error *ngl_invoke(ngl_call * self);

/* Shallow copy the ngl_call. */
ngl_error *ngl_call_clone(ngl_call * dest, ngl_call * src);
/* Delete the internal arrays of the ngl_call. */
ngl_error *ngl_call_deinit(ngl_call * c);
/* Deinitialize and deallocate the call. */
ngl_error *ngl_call_delete(ngl_call * c);
/* Initialize an empty ngl_call. */
void ngl_init_null_call(ngl_call * call);

extern ngl_call ngl_null_call;

/*
 * Macros used to access the ngl_call fields corresponding to a particular
 * wrapped function.
 */
#define ngl_func_to_thunk_name(func_name) \
(join_token(ngl_call_thunk_, func_name))

#define ngl_func_to_thunk(func_name) \
(join_token(ngl_call_, func_name))

#define ngl_func_to_thunk_argc(func_name) \
(join_token(ngl_call_argc_, func_name))

#define ngl_func_to_thunk_args(func_name) \
(join_token(ngl_call_args_, func_name))

#define ngl_func_to_thunk_types(func_name) \
(join_token(ngl_call_types_, func_name))


#endif  // NGL_CALL_H_
