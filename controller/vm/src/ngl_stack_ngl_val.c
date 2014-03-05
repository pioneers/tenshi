#include <ngl_stack_ngl_val.h>
#include <ngl_alloc.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

ngl_define_base(ngl_stack);

/* Entries. */
static const ngl_uint ngl_stack_bucket_default_size = 256;

ngl_stack_bucket *
ngl_stack_bucket_alloc(ngl_uint size, ngl_stack_bucket * prev) {
  if (size < ngl_stack_bucket_default_size) {
    size = ngl_stack_bucket_default_size;
  }
  ngl_stack_bucket *new_bucket = (ngl_stack_bucket *)
    ngl_alloc_simple(char, sizeof(ngl_stack_bucket) +
                     (sizeof(ngl_val) * (size - 1)));
  if (new_bucket != NULL) {
    new_bucket->prev = prev;
    new_bucket->size = size;
    new_bucket->top = 0;
  }
  return new_bucket;
}

ngl_error *
ngl_stack_grow(ngl_stack * self, ngl_uint to_copy, ngl_uint size) {
  ngl_stack_bucket *old_top = self->top;
  ngl_stack_bucket *new_top = ngl_stack_bucket_alloc(size, self->top);
  if (new_top == NULL) {
    return &ngl_out_of_memory;
  } else {
    self->top = new_top;
    memcpy((void *) &self->top->data,
           (void *) (&old_top->data + old_top->top - to_copy),
           to_copy * sizeof(ngl_val));
    return ngl_ok;
  }
}

ngl_error *
ngl_stack_init(ngl_stack * self) {
  self->top = ngl_stack_bucket_alloc(ngl_stack_bucket_default_size, NULL);
  if (self->top == NULL) {
    return &ngl_out_of_memory;
  } else {
    return ngl_ok;
  }
}

static ngl_stack_bucket *
ngl_stack_get_bucket(ngl_stack * self, ngl_uint bucket) {
  ngl_stack_bucket *b = self->top;
  for (size_t i = 0; i < bucket; i++) {
    b = b->prev;
  }
  return b;
}

ngl_error *
ngl_stack_set_size(ngl_stack * self, ngl_uint bucket, ngl_uint size) {
  if (bucket == 0) {
    ngl_stack_bucket *new_bucket =
      ngl_stack_bucket_alloc(size, self->top->prev);
    if (new_bucket == NULL) {
      return &ngl_out_of_memory;
    }
    ngl_stack_bucket *target = self->top;
    memcpy((void *) &new_bucket->data, (void *) &target->data, target->top);
    self->top = new_bucket;
    free(target);
    return ngl_ok;
  }
  ngl_stack_bucket *next_bucket = ngl_stack_get_bucket(self, bucket - 1);
  ngl_stack_bucket *target = next_bucket->prev;
  ngl_stack_bucket *new_bucket = ngl_stack_bucket_alloc(size, target->prev);
  if (new_bucket == NULL) {
    return &ngl_out_of_memory;
  }
  memcpy((void *) &new_bucket->data, (void *) &target->data, target->top);
  next_bucket->prev = new_bucket;
  free(target);
  return ngl_ok;
}

ngl_uint
ngl_stack_get_size(ngl_stack * self, ngl_uint bucket) {
  ngl_stack_bucket *b = ngl_stack_get_bucket(self, bucket);
  return b->size;
}

ngl_error *
ngl_stack_set(ngl_stack * self, ngl_uint idx, ngl_val val) {
  (&self->top->data)[self->top->top - idx] = val;
  return ngl_ok;
}

ngl_val
ngl_stack_get(ngl_stack * self, ngl_uint idx) {
  return (&self->top->data)[self->top->top - idx];
}

ngl_error *
ngl_stack_push(ngl_stack * self, ngl_val val) {
  self->top->top += 1;
  return ngl_stack_set(self, 0, val);
}

ngl_val
ngl_stack_pop(ngl_stack * self) {
  self->top->top -= 1;
  return (&self->top->data)[self->top->top + 1];
}

void
ngl_stack_move(ngl_stack * self, ngl_int diff) {
  self->top->top += diff;
}

ngl_uint
ngl_stack_height(ngl_stack * self) {
  ngl_uint out = 0;
  ngl_stack_bucket *b = self->top;
  while (b != NULL) {
    out += b->top;
    b = b->prev;
  }
  return out;
}

ngl_val *
ngl_stack_get_ptr(ngl_stack * self, ngl_uint idx) {
  return &self->top->data + self->top->top - (ngl_int) idx - 1;
}
