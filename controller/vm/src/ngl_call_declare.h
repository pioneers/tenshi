#ifndef NGL_CALL_DECLARE_H_
#define NGL_CALL_DECLARE_H_
/* This makes cpplint happy.  ^_^ */
#undef NGL_CALL_DECLARE_H_

#ifndef ngl_call_name /* NOLINT(*) */
#error Please define ngl_call_name
#endif

/* NOLINT(*) */
#ifndef ngl_call_argc
#error Please define ngl_call_argc
#endif

#include <ngl_call.h>

ngl_error *ngl_func_to_thunk_name(ngl_call_name) (ngl_val * arg);
extern int ngl_func_to_thunk_argc(ngl_call_name);
extern ngl_call ngl_func_to_thunk(ngl_call_name);
extern ngl_val
ngl_func_to_thunk_args(ngl_call_name)[ngl_call_argc];
     extern ngl_type *ngl_func_to_thunk_types(ngl_call_name)[ngl_call_argc];

#undef ngl_call_name
#undef ngl_call_argc
#endif /* end of include guard: NGL_CALL_DECLARE_H_ */ /* NOLINT(*) */
