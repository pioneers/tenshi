#ifndef NGL_ARRAY_H_
#define NGL_ARRAY_H_
/* This makes cpplint happy.  ^_^ */
#undef NGL_ARRAY_H_

/*
 * This generates an api which is abi compatible with the data structure
 * described in ngl_array_ngl_val.{h, c}. All methods here forward to functions
 * there.
 */

#include <ngl_array_ngl_val.h>
#include <ngl_macros.h>

#ifndef ngl_array_elem_t
#error Must define ngl_array_elem_t.
#endif

#ifndef ngl_array_elem_kind
#error Please define ngl_array_elem_kind
#endif

#ifndef ngl_array_elem_name
#define ngl_array_elem_name ngl_array_elem_t
#endif

#ifndef ngl_array_name
#define ngl_array_name ngl_array_elem_t
#endif


#define ngl_array_elem_ngl_type \
((ngl_type *) & ngl_type_to_ngl_type(ngl_array_elem_name))

#define method(mname) \
join_token(join_token(ngl_array_, ngl_array_name), join_token(_, mname))

#define ngl_array_struct join_token(ngl_array_, ngl_array_name)
#define ngl_array_iter_struct \
join_token(join_token(ngl_array_, ngl_array_name), _iter)

#ifdef pointer
#error pointer should not be defined
#endif

/* 
 * Define pointer so that we can check if ngl_array_elem_kind is defined to be
 * pointer.
 */
/* ngl_array_cast_elem casts an element from the api type to ngl_val. */
#define pointer 0xefaced
#if ngl_array_elem_kind == pointer
#define ngl_array_cast_elem(elem) \
((ngl_val) { .ngl_array_elem_kind = (ngl_obj *) elem })
#else
#define ngl_array_cast_elem(elem) \
((ngl_val) { .ngl_array_elem_kind = elem })
#endif
#undef pointer

/* Cast an element from ngl_val back to the api type. */
#define ngl_array_back_cast(val) ((ngl_array_elem_t)((val).ngl_array_elem_kind))

def_struct(ngl_array_struct, {
  ngl_array array;
})

def_struct(ngl_array_iter_struct, {
  ngl_array_iter iter;
})

/* Initialize an array in place. */
static ngl_error *method(init) (ngl_array_struct * self) {
  return ngl_array_init(&self->array, ngl_array_elem_ngl_type);
}

/* Initialize an array in place with a specific amount of space. */
static ngl_error *method(init_to_size) (ngl_array_struct * self,
                                        int64_t num_elems) {
  return ngl_array_init_to_size(&self->array, ngl_array_elem_ngl_type,
                                num_elems);
}


/* Allocate an array and initialize it. */
static ngl_array_struct *method(new) (void) {
  return (ngl_array_struct *) ngl_array_new(ngl_array_elem_ngl_type);
}

/* Allocate an array and initialize it with a specific amount of space. */
static ngl_array_struct *method(new_to_size) (int64_t num_elems) {
  return (ngl_array_struct *) ngl_array_new_to_size(ngl_array_elem_ngl_type,
                                                    num_elems);
}


/* Deinitialize an array in place. */
static void method(deinit) (ngl_array_struct * self) {
  ngl_array_deinit(&self->array);
}

/* Delete the allocated array. */
static void method(delete) (ngl_array_struct * self) {
  ngl_array_delete(&self->array);
}


/* Check if the array is empty. */
static bool method(empty) (ngl_array_struct * self) {
  return ngl_array_empty(&self->array);
}

/* 
 * Check if the array is full.
 * Note that the array will still resize on insert.
 */
static bool method(full) (ngl_array_struct * self) {
  return ngl_array_full(&self->array);
}


/* Add elem to the back of the array. */
static ngl_error *method(push_back) (ngl_array_struct * self,
                                     ngl_array_elem_t elem) {
  return ngl_array_push_back(&self->array, ngl_array_cast_elem(elem));
}

/* Add elem to the front of the array. */
static ngl_error *method(push_front) (ngl_array_struct * self,
                                      ngl_array_elem_t elem) {
  return ngl_array_push_front(&self->array, ngl_array_cast_elem(elem));
}


/* Remove the back element, and store into <elem>. */
static ngl_error *method(pop_back) (ngl_array_struct * self,
                                    ngl_array_elem_t * elem) {
  ngl_val elem_val;
  ngl_error *error = ngl_array_pop_back(&self->array, &elem_val);
  if (error == ngl_ok) {
    *elem = ngl_array_back_cast(elem_val);
  }
  return error;
}

/* Remove the front element, and store into <elem>. */
static ngl_error *method(pop_front) (ngl_array_struct * self,
                                     ngl_array_elem_t * elem) {
  ngl_val elem_val;
  ngl_error *error = ngl_array_pop_front(&self->array, &elem_val);
  if (error == ngl_ok) {
    *elem = ngl_array_back_cast(elem_val);
  }
  return error;
}


/* Get the length. */
static int64_t method(length) (ngl_array_struct * self) {
  return ngl_array_length(&self->array);
}

/* Check that an index contains an element. */
static bool method(good_index) (ngl_array_struct * self, int64_t index) {
  return ngl_array_good_index(&self->array, index);
}

/* Get a pointer into the array at index <index>. */
static ngl_error *method(index) (ngl_array_struct * self,
                                 int64_t index, ngl_array_elem_t ** elem) {
  ngl_val *elem_val_ptr;
  ngl_error *error = ngl_array_index(&self->array, index, &elem_val_ptr);
  if (error == ngl_ok) {
    *elem = (ngl_array_elem_t *) elem_val_ptr;
  }
  return error;
}

/* Remove the element at index <index>. */
static ngl_error *method(remove) (ngl_array_struct * self, int64_t index) {
  return ngl_array_remove(&self->array, index);
}

/* Store the element at index <index> into <elem>. */
static ngl_error *method(get) (ngl_array_struct * self, int64_t index,
                               ngl_array_elem_t * elem) {
  ngl_val elem_val;
  ngl_error *error = ngl_array_get(&self->array, index, &elem_val);
  if (error == ngl_ok) {
    *elem = ngl_array_back_cast(elem_val);
  }
  return error;
}

/* Set the element at index <index> to <val>. */
static ngl_error *method(set) (ngl_array_struct * self, int64_t index,
                               ngl_array_elem_t val) {
  return ngl_array_set(&self->array, index, ngl_array_cast_elem(val));
}


/* Intialize an iterator to the start of the array. */
static ngl_array_iter_struct method(start) (ngl_array_struct * parent) {
  return (ngl_array_iter_struct) {
  ngl_array_start(&parent->array)} END
}

/* Intialize an iterator to the end of the array. */
static ngl_array_iter_struct method(end) (ngl_array_struct * parent) {
  return (ngl_array_iter_struct) {
  ngl_array_end(&parent->array)} END
}


/* Move an interator back on index. */
static void method(prev) (ngl_array_iter_struct * self) {
  ngl_array_prev(&self->iter);
}

/* Move an interator forward on index. */
static void method(next) (ngl_array_iter_struct * self) {
  ngl_array_next(&self->iter);
}


/* Check that an iterator is still valid. */
static bool method(good) (ngl_array_iter_struct * self) {
  return ngl_array_good(&self->iter);
}

/* Get the element from the iterator. */
static ngl_array_elem_t method(deref) (ngl_array_iter_struct * self) {
  return ngl_array_back_cast(ngl_array_deref(&self->iter));
}

/* Get an element relative to the current index of the iterator. */
static ngl_array_elem_t *method(at) (ngl_array_iter_struct * self,
                                     int64_t rel_idx) {
  return (ngl_array_elem_t *) ngl_array_at(&self->iter, rel_idx);
}

/*
 * Check that an index relative to the current index of the iterator is valid.
 */
static bool method(good_at) (ngl_array_iter_struct * self, int64_t rel_idx) {
  return ngl_array_good_at(&self->iter, rel_idx);
}

/*
 * Remove an element at an index relative to the current index of the iterator.
 */
static ngl_error *method(remove_at) (ngl_array_iter_struct * self,
                                     int64_t rel_idx) {
  return ngl_array_remove_at(&self->iter, rel_idx);
}




static int64_t method(index_of) (ngl_array_struct * self,
                                 ngl_array_elem_t to_find) {
  return ngl_array_index_of(&self->array, ngl_array_cast_elem(to_find));
}

#undef method
#undef ngl_array_back_cast
#undef ngl_array_cast_elem
#undef ngl_array_elem_kind
#undef ngl_array_elem_name
#undef ngl_array_elem_t
#undef ngl_array_elem_ngl_type
#undef ngl_array_iter_struct
#undef ngl_array_name
#undef ngl_array_struct

#endif  // NGL_ARRAY_H_
