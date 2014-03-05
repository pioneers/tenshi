#ifndef NGL_THREAD_H
#define NGL_THREAD_H
#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_error.h>
#include <ngl_stack_ngl_val.h>

def_struct ( ngl_thread ) {
  /* TODO(kzentner): Store the current opbunch here. */
  ngl_obj header;
  ngl_stack stack;
  ngl_stack call_stack;
  };

ngl_error * ngl_thread_init ( ngl_thread * thread );

ngl_declare_composite ( ngl_thread );

#endif /* end of include guard: NGL_THREAD_H */
