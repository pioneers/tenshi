#ifndef NGL_STATE_H
#define NGL_STATE_H
#include <ngl_macros.h>

def_struct ( ngl_vm ) {
  opbunch * func;
  opbunch * pc;
  ngl_stack stack;
  ngl_stack call_stack;
  };

#endif /* end of include guard: NGL_STATE_H */
