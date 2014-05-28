#ifndef NGL_THREAD_H_
#define NGL_THREAD_H_

#include <ngl_types_wrapper.h>

#include <ngl_error.h>
#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_stack_ngl_val.h>

ngl_error *ngl_thread_init(ngl_thread * thread);

ngl_declare_composite(ngl_thread);

#endif  // NGL_THREAD_H_
