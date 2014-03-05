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

#include <ngl_call.h>

#define NGL_ARG_INT(idx, type) ((type) arg[idx].integer)
#define NGL_ARG_UINT(idx, type) ((type) arg[idx].uinteger)
#define NGL_ARG_FLOAT(idx, type) ((type) arg[idx].floating)
#define NGL_ARG_PTR(idx, type) ((type) arg[idx].pointer)

/* Define the associated variables and thunk. */

ngl_error *ngl_func_to_thunk_name(ngl_call_name) (ngl_val * arg) {
  /* 
   * ngl_call_args expands to multiple arguments of the form
   * NGL_ARG_INT(0, int), NGL_ARG_FLOAT(1, double), etc. This causes the C
   * compiler to output appropriate conversion code.
   */
  return ngl_call_name(ngl_call_args);
}

int ngl_func_to_thunk_argc(ngl_call_name) = ngl_call_argc;
ngl_call ngl_func_to_thunk(ngl_call_name);
ngl_val
ngl_func_to_thunk_args(ngl_call_name)[ngl_call_argc];
ngl_type *ngl_func_to_thunk_types(ngl_call_name)[ngl_call_argc];

#undef ngl_call_name
#undef ngl_call_args
#undef ngl_call_argc
