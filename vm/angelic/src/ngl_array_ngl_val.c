#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ngl_type.h>
#include <ngl_bits.h>
#include <ngl_macros.h>
#include <ngl_error.h>
#include <ngl_alloc.h>

#include <ngl_array_ngl_val.h>

ngl_define_base(ngl_array);

static const ngl_int ngl_array_default_size = 8;
ngl_error ngl_array_no_elem;

ngl_error *
ngl_array_init(ngl_array * self, ngl_type * ngl_array_elem_type) {
  return ngl_array_init_to_size(self, ngl_array_elem_type,
                                ngl_array_default_size);
}

ngl_error *
ngl_array_init_to_size(ngl_array * self,
                       ngl_type * ngl_array_elem_type, ngl_int num_elements) {
  self->ngl_array_elem_type = ngl_array_elem_type;
  self->space = num_elements;
  self->start = 0;
  self->past_end = 0;

  self->data = ngl_alloc_simple(ngl_val, self->space);

  if (!self->data) {
    return &ngl_out_of_memory;
  } else {
    return ngl_ok;
  }
}


ngl_array *
ngl_array_new(ngl_type * ngl_array_elem_type) {
  return ngl_array_new_to_size(ngl_array_elem_type, ngl_array_default_size);
}

ngl_array *
ngl_array_new_to_size(ngl_type * ngl_array_elem_type, ngl_int num_elements) {
  ngl_array *self = ngl_alloc_simple(ngl_array, 1);
  if (!self) {
    return NULL;
  }
  if (ngl_array_init_to_size(self,
          ngl_array_elem_type,
          num_elements) != ngl_ok) {
    free(self);
    return NULL;
  }
  return self;
}


void
ngl_array_deinit(ngl_array * self) {
  free(self->data);
}

void
ngl_array_delete(ngl_array * self) {
  ngl_array_deinit(self);
  free(self);
}


static bool
ngl_array_looped(ngl_array * self) {
  return self->start > self->past_end;
}


bool
ngl_array_grow(ngl_array * self) {
  ngl_int new_size = ngl_round_up_power_2(self->space + 1);
  ngl_val *new_data = ngl_alloc_simple(ngl_val, new_size);
  if (!new_data) {
    return false;
  }
  if (ngl_array_looped(self)) {
    ngl_int end_size = self->space - self->start;
    memcpy((void *) new_data,
           (void *) (self->data + self->start), end_size * sizeof(ngl_val));
    memcpy((void *) (new_data + self->start),
           (void *) self->data, self->past_end * sizeof(ngl_val));
    self->past_end = end_size + self->past_end;
  } else {
    memcpy((void *) new_data,
           (void *) (self->data + self->start),
           sizeof(ngl_val) * (self->past_end - self->start));
  }
  self->start = 0;
  free(self->data);
  self->data = new_data;
  self->space = new_size;
  return true;
}

bool
ngl_array_empty(ngl_array * self) {
  return self->past_end == 0;
}

bool
ngl_array_full(ngl_array * self) {
  return (self->past_end != 0 && self->start == self->past_end)
    || (self->start == 0 && self->past_end == self->space);
}

ngl_error *
ngl_array_push_back(ngl_array * self, ngl_val elem) {
  if (ngl_array_full(self)) {
    if (!ngl_array_grow(self)) {
      return &ngl_out_of_memory;
    }
  }
  if (self->past_end == self->space) {
    /* Handle past_end rolling forwards. */
    self->past_end = 1;
    self->data[0] = elem;
  } else {
    self->data[self->past_end++] = elem;
  }
  return ngl_ok;
}

ngl_error *
ngl_array_push_front(ngl_array * self, ngl_val elem) {
  if (ngl_array_full(self)) {
    ngl_array_grow(self);
  }
  if (!ngl_array_full(self)) {
    if (ngl_array_empty(self)) {
      /* Handle empty ngl_arrays. */
      self->past_end = 1;
    } else if (self->start == 0) {
      /* Handle start rolling backwards. */
      self->start = self->space - 1;
    } else {
      --self->start;
    }
    self->data[self->start] = elem;
  } else {
    return &ngl_out_of_memory;
  }
  return ngl_ok;
}

ngl_error *
ngl_array_pop_back(ngl_array * self, ngl_val * elem) {
  if (ngl_array_empty(self)) {
    return &ngl_array_no_elem;
  }

  *elem = self->data[--self->past_end];

  if (self->past_end == 1 && self->start != 0) {
    /*
     * Handle past_end rolling backwards.
     * Implies the ngl_array is not becoming empty.
     */
    self->past_end = self->space;
  } else {
    /* Handles the normal case and the ngl_array becoming empty. */
    if (self->past_end == self->start) {
      /* Array is emptied. */
      self->past_end = 0;
      self->start = 0;
    }
  }
  return ngl_ok;
}

ngl_error *
ngl_array_pop_front(ngl_array * self, ngl_val * elem) {
  if (ngl_array_empty(self)) {
    return &ngl_array_no_elem;
  }

  *elem = self->data[self->start++];

  if (self->start == self->space && self->past_end != self->space) {
    /*
     * Handle start rolling forwards.
     * Implies the ngl_array is not becoming empty.
     */
    self->start = 0;
  } else if (self->start == self->past_end) {
    /* Array is empitied. */
    self->past_end = 0;
  }
  return ngl_ok;
}


ngl_int
ngl_array_length(ngl_array * self) {
  if (ngl_array_looped(self)) {
    ngl_int end_size = self->space - self->start;
    ngl_int front_size = self->past_end;
    return end_size + front_size;
  } else {
    return self->past_end - self->start;
  }
}

bool
ngl_array_good_index(ngl_array * self, ngl_int index) {
  return index < ngl_array_length(self) && index >= -ngl_array_length(self);
}

ngl_error *
ngl_array_index(ngl_array * self, ngl_int index, ngl_val ** elem) {
  if (index < 0) {
    index = ngl_array_length(self) + index;
  }
  if (ngl_array_looped(self)) {
    ngl_int end_size = self->space - self->start;
    if (index < end_size) {
      /* Index is in the loop-back end segment. */
      *elem = self->data + self->start + index;
    } else if (index - end_size < (ssize_t) self->past_end) {
      /* Index is in the normal segment. */
      *elem = self->data + (index - end_size);
    } else {
      /* Index is out of bounds(between the segments). */
      return &ngl_array_no_elem;
    }
  } else {
    if (index + self->start < self->past_end) {
      /* Index is in bounds. */
      *elem = self->data + self->start + index;
    } else {
      /* Index is out of bounds. */
      return &ngl_array_no_elem;
    }
  }
  return ngl_ok;
}

ngl_error *
ngl_array_remove(ngl_array * self, ngl_int index) {
  if (index < 0) {
    index = ngl_array_length(self) + index;
  }
  if (ngl_array_looped(self)) {
    ngl_int end_size = self->space - self->start;
    if (index < end_size) {
      /* Index is in the loop-back end segment. */
      void *target = (void *) (self->data + self->start + 1);
      void *source = (void *) (self->data + self->start);
      memmove(target, source, sizeof(ngl_val) * index);
      self->start += 1;
      if (self->start == self->space) {
        self->start = 0;
      }
    } else if (index - end_size < (ssize_t) self->past_end) {
      /* Index is in the normal segment. */
      ngl_int sindex = index - end_size;
      void *target = (void *) (self->data + sindex);
      void *source = (void *) (self->data + sindex + 1);
      size_t size = self->past_end - sindex - 1;
      memmove(target, source, sizeof(ngl_val) * size);
      self->past_end -= 1;
      if (self->past_end == 0) {
        self->past_end = self->space;
      }
    } else {
      /* Index is out of bounds(between the segments). */
      return &ngl_array_no_elem;
    }
  } else {
    if (index + self->start < self->past_end) {
      /* Index is in bounds. */
      size_t sindex = self->start + index;
      void *target = (void *) (self->data + sindex);
      void *source = (void *) (self->data + sindex + 1);
      size_t size = self->past_end - sindex - 1;
      memmove(target, source, sizeof(ngl_val) * size);
      self->past_end -= 1;
      if (self->start == self->past_end) {
        self->past_end = 0;
        self->start = 0;
      }
    } else {
      /* Index is out of bounds. */
      return &ngl_array_no_elem;
    }
  }
  return ngl_ok;
}

ngl_error *
ngl_array_get(ngl_array * self, ngl_int index, ngl_val * elem) {
  ngl_val *elem_ptr;
  ngl_error *e = ngl_array_index(self, index, &elem_ptr);
  if (e == ngl_ok) {
    *elem = *elem_ptr;
  }
  return e;
}

ngl_error *
ngl_array_set(ngl_array * self, ngl_int index, ngl_val val) {
  ngl_val *elem_ptr;
  ngl_error *e = ngl_array_index(self, index, &elem_ptr);
  if (e == ngl_ok) {
    *elem_ptr = val;
  }
  return e;
}


ngl_array_iter
ngl_array_start(ngl_array * parent) {
  return ((ngl_array_iter) {
  .parent = parent, .index = 0});
}

ngl_array_iter
ngl_array_end(ngl_array * parent) {
  return ((ngl_array_iter) {
  .parent = parent, .index = -1});
}


void
ngl_array_next(ngl_array_iter * self) {
  self->index += 1;
}

void
ngl_array_prev(ngl_array_iter * self) {
  self->index -= 1;
}


bool
ngl_array_good(ngl_array_iter * self) {
  return ngl_array_good_index(self->parent, self->index);
}

ngl_val
ngl_array_deref(ngl_array_iter * self) {
  ngl_val elem;
  ngl_error *e = ngl_array_get(self->parent, self->index, &elem);
  if (e == ngl_ok) {
    return elem;
  } else {
    assert(false);
  }
  return ngl_val_int(0);
}

ngl_val *
ngl_array_at(ngl_array_iter * self, ngl_int rel_idx) {
  ngl_val *elem_ptr;
  ngl_error *e =
    ngl_array_index(self->parent, self->index + rel_idx, &elem_ptr);
  if (e == ngl_ok) {
    return elem_ptr;
  } else {
    return NULL;
  }
}

bool
ngl_array_good_at(ngl_array_iter * self, ngl_int rel_idx) {
  if (self->index < 0) {
    return rel_idx + self->index < 0 &&
      rel_idx + self->index >= -ngl_array_length(self->parent);
  } else {
    return rel_idx + self->index >= 0 &&
      rel_idx + self->index < ngl_array_length(self->parent);
  }
}

ngl_error *
ngl_array_remove_at(ngl_array_iter * self, ngl_int rel_idx) {
  return ngl_array_remove(self->parent, self->index + rel_idx);
}


ngl_int
ngl_array_index_of(ngl_array * self, ngl_val to_find) {
  ngl_array_iter i = ngl_array_start(self);
  while (ngl_array_good(&i)) {
    if (to_find.integer == ngl_array_deref(&i).integer) {
      return i.index;
    }
    ngl_array_next(&i);
  }
  return -1;
}
