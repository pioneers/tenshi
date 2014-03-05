#include <assert.h>
#include <string.h>
#include <ngl_table_ngl_val.h>
#include <ngl_bits.h>
#include <ngl_alloc.h>
#include <ngl_str.h>
#include <ngl_val.h>
#include <ngl_type.h>

ngl_error ngl_table_not_found;

bool ngl_table_needs_grow ( ngl_table * self );

bool ngl_table_needs_shrink ( ngl_table * self );

ngl_error * ngl_table_grow ( ngl_table * restrict self );

#define SELECT_BUCKET( \
    macro_self, \
    macro_key, \
    macro_hashed, \
    macro_buckets, \
    macro_ahen_first, \
    macro_ahen_not_first, \
    macro_ahen_missing_first, \
    macro_ahen_missing_not_first ) \
  do { \
    const ngl_hash hashed = macro_hashed; \
    ngl_table_bucket * macro_the_bucket = ( macro_buckets ) + ( ( hashed ) & ( macro_self )->mask ); \
    if ( ! macro_the_bucket->hash ) { \
      macro_ahen_missing_first ( macro_the_bucket ); \
      } \
    else if ( macro_self->interface->compare ( macro_the_bucket->key, macro_key ) == 0 ) { \
      macro_ahen_first ( macro_the_bucket ); \
      } \
    else { \
      while ( macro_the_bucket->next ) { \
        macro_the_bucket = macro_the_bucket->next; \
        if ( macro_self->interface->compare ( macro_the_bucket->key, macro_key ) == 0 ) { \
          macro_ahen_not_first ( macro_the_bucket ); \
          } \
        } \
      macro_ahen_missing_not_first ( macro_the_bucket ); \
      } \
    } while ( 0 )

const size_t ngl_table_default_size = 8;

size_t ngl_table_count_elems ( ngl_table * self ) {
  size_t out = 0;
  for ( size_t i = 0; i < self->space; i++ ) {
    ngl_table_bucket * b = self->data + i;
    while ( b ) {
      if (b->hash) {
        out += 1;
        }
      b = b->next;
      }
    }
  return out;
  }

ngl_error * ngl_table_init_to_size (
    ngl_table * self,
    ngl_type * key_type,
    ngl_type * val_type,
    size_t predicted_elems,
    ngl_table_elem_interface * hash_interface
    ) {
  assert ( self );
  predicted_elems = ngl_round_up_power_2 ( predicted_elems );
  self->key_type = key_type;
  self->val_type = val_type;
  self->interface = hash_interface;
  self->mask = ngl_mask_of_pow2 ( predicted_elems );
  self->space = predicted_elems;
  self->num_elems = 0;
  self->data = ngl_alloc_simple ( ngl_table_bucket, predicted_elems );
  if ( self->data == NULL ) {
    return &ngl_out_of_memory;
    }
  else {
    memset ( ( void * ) self->data, 0, predicted_elems * sizeof ( ngl_table_bucket ) );
    return ngl_ok;
    }
  }

ngl_error * ngl_table_init (
    ngl_table * self,
    ngl_type * key_type,
    ngl_type * val_type,
    ngl_table_elem_interface * hash_interface
    ) {
  return ngl_table_init_to_size (
      self,
      key_type,
      val_type,
      ngl_table_default_size,
      hash_interface );
  }

ngl_table * ngl_table_new (
    ngl_type * key_type,
    ngl_type * val_type,
    ngl_table_elem_interface * hash_interface
  ) {
  ngl_table * self = ngl_alloc_simple ( ngl_table, 1 );
  if ( ! self ) {
    return NULL;
    }
  if ( ngl_table_init ( self,
                     key_type,
                     val_type,
		     hash_interface ) == ngl_ok ) {
    return self;
    }
  else {
    free ( self );
    return NULL;
    }
  }

ngl_error * ngl_table_get ( ngl_table * self, ngl_val key, ngl_val * dst ) {
  return ngl_table_get_hash ( self, key, dst, self->interface->hash ( key ) );
  }
ngl_error * ngl_table_get_hash ( ngl_table * self, ngl_val key, ngl_val * dst, ngl_hash hash ) {
  #define SET_DST( bucket ) \
    *dst = bucket->value; \
    return ngl_ok;
  #define RETURN_ERROR( bucket ) \
    return &ngl_table_not_found;
  SELECT_BUCKET ( self,
                  key,
                  hash,
                  self->data,
                  SET_DST,
                  SET_DST,
                  RETURN_ERROR,
                  RETURN_ERROR );
  #undef SET_DST
  #undef RETURN_ERROR
  }

ngl_error * ngl_table_get_or_add (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_call * on_missing
    ) {
  return ngl_table_get_or_add_hash (
      self,
      key,
      dst,
      on_missing,
      self->interface->hash ( key ) );
  }
ngl_error * ngl_table_get_or_add_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_call * on_missing,
    ngl_hash hash
    ) {
    ngl_error * e = ngl_ok;
  #define RETURN_FOUND( bucket ) \
    *dst = ( bucket )->value; \
    return ngl_ok;
  #define ADD_FIRST( bucket ) \
    ( bucket )->hash = hashed; \
    ( bucket )->key = key; \
    ngl_call_push ( on_missing, self->key_type->ptr_of, (ngl_val) { .pointer = ( ngl_obj * ) &( bucket )->key } ); \
    ngl_call_push ( on_missing, self->val_type->ptr_of, (ngl_val) { .pointer = ( ngl_obj * ) &( bucket )->value } ); \
    if ( ngl_invoke ( on_missing ) != ngl_ok ) { \
      ngl_val macro_value = ( bucket )->value; \
      ( bucket )->hash = 0; \
      *dst = macro_value; \
      return ngl_ok; \
      } \
    self->num_elems++; \
    if ( ngl_table_needs_grow ( self ) ){ \
      e = ngl_table_grow ( self ); \
      } \
    *dst = ( bucket )->value; \
    return e;
  #define ADD_NOT_FIRST( bucket ) \
    ngl_table_bucket * new_bucket = ngl_alloc_simple ( ngl_table_bucket, 1 ); \
    if ( new_bucket == NULL ) { \
      return &ngl_out_of_memory; \
      } \
    ( bucket )->next = new_bucket;  \
    new_bucket->next = NULL; \
    ( new_bucket )->hash = hashed; \
    ( new_bucket )->key = key; \
    ngl_call_push ( on_missing, self->key_type->ptr_of, (ngl_val) { .pointer = ( ngl_obj * ) &( new_bucket )->key } ); \
    ngl_call_push ( on_missing, self->val_type->ptr_of, (ngl_val) { .pointer = ( ngl_obj * ) &( new_bucket )->value } ); \
    e = ngl_invoke ( on_missing ); \
    if ( e != ngl_ok ) { \
      free ( new_bucket ); \
      ( bucket )->next = NULL; \
      return e; \
      } \
    self->num_elems++; \
    *dst = ( new_bucket )->value; \
    if ( ngl_table_needs_grow ( self ) ){ \
      return ngl_table_grow ( self ); \
      } \
    return ngl_ok;
  SELECT_BUCKET (
      self,
      key,
      hash,
      self->data,
      RETURN_FOUND,
      RETURN_FOUND,
      ADD_FIRST,
      ADD_NOT_FIRST );
      #undef ADD_NOT_FIRST
  }

/* Note that these two functions share most of their implementations. */

ngl_error * ngl_table_get_default (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_val default_value
    ) {
  return ngl_table_get_default_hash (
      self,
      key,
      dst,
      default_value,
      self->interface->hash ( key ) );
  }

ngl_error * ngl_table_get_default_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val * dst,
    ngl_val default_value,
    ngl_hash hash
    ) {
  #undef ADD_FIRST
  #define ADD_FIRST( bucket ) \
    self->num_elems++; \
    ( bucket )->hash = hashed; \
    ( bucket )->key = key; \
    *dst = ( bucket )->value = default_value; \
    if ( ngl_table_needs_grow ( self ) ){ \
      return ngl_table_grow ( self ); \
      } \
    return ngl_ok;
  #define ADD_NOT_FIRST( bucket ) \
    ngl_table_bucket * new_bucket = ngl_alloc_simple ( ngl_table_bucket, 1 ); \
    if ( new_bucket == NULL ) { \
      return &ngl_out_of_memory; \
      } \
    ( bucket )->next = new_bucket;  \
    new_bucket->next = NULL; \
    ADD_FIRST ( new_bucket );
  SELECT_BUCKET (
      self,
      key,
      hash,
      self->data,
      RETURN_FOUND,
      RETURN_FOUND,
      ADD_FIRST,
      ADD_NOT_FIRST );
  #undef RETURN_FOUND
  }

ngl_error * ngl_table_set (
    ngl_table * self,
    ngl_val key,
    ngl_val value
    ) {
  return ngl_table_set_hash (
      self,
      key,
      value,
      self->interface->hash ( key ) );
  }
ngl_error * ngl_table_set_hash (
    ngl_table * self,
    ngl_val key,
    ngl_val value,
    ngl_hash hash
    ) {
  #undef ADD_FIRST
  #define ADD_FIRST( bucket ) \
    self->num_elems++; \
    assert ( bucket ); \
    ( bucket )->hash = hashed; \
    ( bucket )->key = key; \
    ( bucket )->value = value; \
    if ( ngl_table_needs_grow ( self ) ){ \
      return ngl_table_grow ( self ); \
      } \
    return ngl_ok;
  #define SET( bucket ) \
    ( bucket )->value = value; \
    return ngl_ok;
  SELECT_BUCKET (
      self,
      key,
      hash,
      self->data,
      SET,
      SET,
      ADD_FIRST,
      ADD_NOT_FIRST );
  #undef ADD_NOT_FIRST
  #undef SET
  }

bool ngl_table_needs_grow ( ngl_table * self ) {
  return self->num_elems >= (self->space >> 1);
  }

bool ngl_table_needs_shrink ( ngl_table * self ) {
  return self->num_elems <= (self->space >> 2);
  }

ngl_error * ngl_table_delete ( ngl_table * self ) {
  if ( self ) {
    free ( self->data );
    }
  free ( self );
  return ngl_ok;
  }

ngl_error * ngl_table_grow ( ngl_table * restrict self ) {
  ngl_table_bucket * new_buckets = ngl_alloc_simple ( ngl_table_bucket, self->space << 1 );
  if ( new_buckets == NULL ) {
    return &ngl_out_of_memory;
    }
  memset ( ( void * ) new_buckets, 0, (self->space << 1) * sizeof ( ngl_table_bucket ) );
  ngl_table_bucket * old_bucket = self->data;
  self->mask = (self->mask << 1) | 1;
  for ( size_t i = 0; i < self->space; i++ ) {
    old_bucket = self->data + i;
    if ( old_bucket->hash ) {
      assert ( old_bucket->hash != 1 );
      #define IGNORE( bucket ) {}
      #define IMPOSSIBLE( bucket ) assert(0);
      #define MOVE_FIRST( bucket ) \
        memcpy ( bucket, old_bucket, sizeof ( ngl_table_bucket ) ); \
        ( bucket )->next = 0;
      SELECT_BUCKET (
          self,
          old_bucket->key,
          old_bucket->hash,
          new_buckets,
          IMPOSSIBLE,
          IMPOSSIBLE,
          MOVE_FIRST,
          IMPOSSIBLE );
      old_bucket = old_bucket->next;
      while ( old_bucket ) {
        #define MOVE_AND_FREE( bucket ) \
          memcpy ( bucket, old_bucket, sizeof ( ngl_table_bucket ) ); \
          free ( old_bucket ); \
          old_bucket = ( bucket )->next; \
          ( bucket )->next = 0;
        #define RELINK( bucket ) \
          ( bucket )->next = old_bucket; \
          old_bucket = old_bucket->next; \
          ( bucket )->next->next = 0;
        SELECT_BUCKET (
            self,
            old_bucket->key,
            old_bucket->hash,
            new_buckets,
            IMPOSSIBLE,
            IMPOSSIBLE,
            MOVE_AND_FREE,
            RELINK );
        }
      }
    }
  self->space <<= 1;
  free ( self->data );
  self->data = new_buckets;
  assert ( self->num_elems == ngl_table_count_elems ( self ) );
  return ngl_ok;
  }


ngl_table_elem_interface ngl_table_ngl_val_interface = {
  &ngl_hash_ngl_val,
  &ngl_compare_ngl_val
  };
