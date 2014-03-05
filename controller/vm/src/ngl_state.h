#ifndef NGL_STATE_H_
#define NGL_STATE_H_
#include <ngl_macros.h>

def_struct(ngl_vm, {
  opbunch *func;
  opbunch *pc;
  ngl_stack stack;
  ngl_stack call_stack;
})

#endif /* end of include guard: NGL_STATE_H_ */ /* NOLINT(*) */
