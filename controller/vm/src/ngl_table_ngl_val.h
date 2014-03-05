#ifndef NGL_TABLE_H
#define NGL_TABLE_H
#include <stdbool.h>
#include <stdlib.h>
#include <ngl_call.h>
#include <ngl_macros.h>
#include <ngl_type_h.h>
#include <ngl_hash.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>

extern ngl_error ngl_table_not_found;

def_struct (ngl_table_elem_interface) {
  ngl_hash_func_t * hash;
  ngl_hash_compare_func_t * compare;
  };

def_struct (ngl_table_bucket) {
  ngl_val key;
  ngl_val value;
  ngl_table_bucket * next;
  ngl_hash hash;
  };

def_struct (ngl_table) {
  ngl_obj header;
  ngl_table_elem_interface * interface;
  ngl_type * key_type;
  ngl_type * val_type;
  size_t mask;
  size_t num_elems;
  size_t space;
  ngl_table_bucket * data;
  };

ngl_table * ngl_table_new (
    ngl_type * key_type,
    ngl_type * val_type,
    ngl_table_elem_interface * hash_interface
  );

ngl_error * ngl_table_init_to_size (
    ngl_table * self,
    ngl_type * key_type,
    ngl_type * val_type,
    size_t predicted_elems,
    ngl_table_elem_interface * hash_interface);

ngl_error * ngl_table_init (
    ngl_table * self,
    ngl_type * key_type,
    ngl_type * val_type,
    ngl_table_elem_interface * hash_interface);

ngl_error * ngl_table_get (ngl_table * self, ngl_val key, ngl_val * dst);
ngl_error * ngl_table_get_hash (ngl_table * self, ngl_val key, ngl_val * dst, ngl_hash hash);


ngl_error * ngl_table_get_or_add (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_call * on_missing);

ngl_error * ngl_table_get_or_add_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_call * on_missing,
    ngl_hash hash);

ngl_error * ngl_table_get_default (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_val default_value);
ngl_error * ngl_table_get_default_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_val default_value,
    ngl_hash hash);

ngl_error * ngl_table_set (
    ngl_table * self,
    ngl_val key,
    ngl_val value);
ngl_error * ngl_table_set_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val value,
    ngl_hash hash);

ngl_error * ngl_table_delete (ngl_table * self);

extern ngl_table_elem_interface ngl_table_ngl_val_interface;

#endif /* end of include guard: NGL_TABLE_H */
