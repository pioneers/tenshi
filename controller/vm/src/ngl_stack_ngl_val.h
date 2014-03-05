#ifndef NGL_STACK_NGL_VAL_H_
#define NGL_STACK_NGL_VAL_H_
#include <ngl_val.h>
#include <ngl_error.h>

def_struct(ngl_stack_bucket, {
  ngl_stack_bucket *prev;
  ngl_uint size;
  ngl_uint top;
  ngl_val data;
})

def_struct(ngl_stack, {
  ngl_stack_bucket *top_bucket;
})

ngl_error *ngl_stack_init(ngl_stack * self);
ngl_error *ngl_stack_grow(ngl_stack * self, ngl_uint to_copy, ngl_uint size);
ngl_error *ngl_stack_set_size(ngl_stack * self, ngl_uint bucket, ngl_uint size);
ngl_uint ngl_stack_get_size(ngl_stack * self, ngl_uint bucket);
ngl_val ngl_stack_get(ngl_stack * self, ngl_uint idx);
ngl_error *ngl_stack_set(ngl_stack * self, ngl_uint idx, ngl_val val);
ngl_error *ngl_stack_push(ngl_stack * self, ngl_val val);
ngl_val ngl_stack_pop(ngl_stack * self);
ngl_uint ngl_stack_height(ngl_stack * self);
ngl_val *ngl_stack_get_ptr(ngl_stack * self, ngl_uint idx);
void ngl_stack_move(ngl_stack * self, ngl_int diff);

#endif /* end of include guard: NGL_STACK_NGL_VAL_H_ */ /* NOLINT(*) */
