#ifndef NGL_ARRAY_NGL_VAL_H_
#define NGL_ARRAY_NGL_VAL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ngl_type_h.h>
#include <ngl_val.h>
#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_error.h>

/*
 * A looping, growing array.
 */

def_struct(ngl_array, {
  ngl_obj header;
  ngl_type *ngl_array_elem_type;
  size_t space;
  size_t past_end;
  size_t start;
  ngl_val *data;
})

def_struct(ngl_array_iter, {
  ngl_type *type;
  ngl_array *parent;
  ngl_int index;
})

ngl_declare_base(ngl_array);

ngl_error *ngl_array_init(ngl_array * self, ngl_type * ngl_array_elem_type);
ngl_error *ngl_array_init_to_size(ngl_array * self,
                                  ngl_type * ngl_array_elem_type,
                                  ngl_int num_elements);

ngl_array *ngl_array_new(ngl_type * ngl_array_elem_type);
ngl_array *ngl_array_new_to_size(ngl_type * ngl_array_elem_type,
                                 ngl_int num_elements);

void ngl_array_deinit(ngl_array * self);
void ngl_array_delete(ngl_array * self);

bool ngl_array_empty(ngl_array * self);
bool ngl_array_full(ngl_array * self);

ngl_error *ngl_array_push_back(ngl_array * self, ngl_val elem);
ngl_error *ngl_array_push_front(ngl_array * self, ngl_val elem);

ngl_error *ngl_array_pop_back(ngl_array * self, ngl_val * elem);
ngl_error *ngl_array_pop_front(ngl_array * self, ngl_val * elem);

ngl_int ngl_array_length(ngl_array * self);
bool ngl_array_good_index(ngl_array * self, ngl_int index);
ngl_error *ngl_array_index(ngl_array * self, ngl_int index, ngl_val ** elem);
ngl_error *ngl_array_remove(ngl_array * self, ngl_int index);
ngl_error *ngl_array_get(ngl_array * self, ngl_int index, ngl_val * elem);
ngl_error *ngl_array_set(ngl_array * self, ngl_int index, ngl_val val);

ngl_array_iter ngl_array_start(ngl_array * parent);
ngl_array_iter ngl_array_end(ngl_array * parent);

void ngl_array_prev(ngl_array_iter * self);
void ngl_array_next(ngl_array_iter * self);

bool ngl_array_good(ngl_array_iter * self);
ngl_val ngl_array_deref(ngl_array_iter * self);
ngl_val *ngl_array_at(ngl_array_iter * self, ngl_int rel_idx);
bool ngl_array_good_at(ngl_array_iter * self, ngl_int rel_idx);
ngl_error *ngl_array_remove_at(ngl_array_iter * self, ngl_int rel_idx);


ngl_int ngl_array_index_of(ngl_array * self, ngl_val to_find);

#endif /* end of include guard: NGL_ARRAY_NGL_VAL_H_ */ /* NOLINT(*) */
