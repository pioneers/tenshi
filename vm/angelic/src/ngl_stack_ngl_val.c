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
    /* Top is the last value written, so it's initially -1. */
    new_bucket->top = -1;
  }
  return new_bucket;
}

ngl_error *
ngl_stack_grow(ngl_stack * self, ngl_uint to_copy, ngl_uint size) {
  ngl_stack_bucket *old_top = self->top_bucket;
  ngl_stack_bucket *new_top = ngl_stack_bucket_alloc(size, self->top_bucket);
  if (new_top == NULL) {
    return &ngl_out_of_memory;
  } else {
    self->top_bucket = new_top;
    memcpy((void *) &self->top_bucket->data,
           (void *) (&old_top->data + old_top->top - to_copy),
           to_copy * sizeof(ngl_val));
    return ngl_ok;
  }
}

ngl_error *
ngl_stack_init(ngl_stack * self) {
  self->top_bucket = ngl_stack_bucket_alloc(ngl_stack_bucket_default_size,
                                            NULL);
  if (self->top_bucket == NULL) {
    return &ngl_out_of_memory;
  } else {
    return ngl_ok;
  }
}

static ngl_stack_bucket *
ngl_stack_get_bucket(ngl_stack * self, ngl_uint bucket) {
  ngl_stack_bucket *b = self->top_bucket;
  for (size_t i = 0; i < bucket; i++) {
    b = b->prev;
  }
  return b;
}

ngl_error *
ngl_stack_set_size(ngl_stack * self, ngl_uint bucket, ngl_uint size) {
  if (bucket == 0) {
    ngl_stack_bucket *new_bucket =
      ngl_stack_bucket_alloc(size, self->top_bucket->prev);
    if (new_bucket == NULL) {
      return &ngl_out_of_memory;
    }
    ngl_stack_bucket *target = self->top_bucket;
    memcpy((void *) &new_bucket->data, (void *) &target->data, target->top);
    self->top_bucket = new_bucket;
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
  *ngl_stack_get_ptr(self, idx) = val;
  return ngl_ok;
}

ngl_val
ngl_stack_get(ngl_stack * self, ngl_uint idx) {
  return *ngl_stack_get_ptr(self, idx);
}

ngl_error *
ngl_stack_push(ngl_stack * self, ngl_val val) {
  self->top_bucket->top += 1;
  return ngl_stack_set(self, 0, val);
}

ngl_val
ngl_stack_pop(ngl_stack * self) {
  ngl_val ret = ngl_stack_get(self, 0);
  self->top_bucket->top -= 1;
  return ret;
}

void
ngl_stack_move(ngl_stack * self, ngl_int diff) {
  self->top_bucket->top += diff;
}

ngl_uint
ngl_stack_height(ngl_stack * self) {
  ngl_int out = 0;
  ngl_stack_bucket *b = self->top_bucket;
  while (b != NULL) {
    out += b->top + 1;
    b = b->prev;
  }
  return out;
}

ngl_val *
ngl_stack_get_ptr(ngl_stack * self, ngl_uint idx) {
  return &self->top_bucket->data + self->top_bucket->top - (ngl_int) idx;
}
