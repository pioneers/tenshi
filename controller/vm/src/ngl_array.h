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


#define ngl_array_elem_ngl_type ((ngl_type *) & ngl_type_to_ngl_type ( ngl_array_elem_name ))

#define method( mname ) join_token ( join_token ( ngl_array_, ngl_array_name ), join_token ( _, mname ) )

#define ngl_array_struct join_token ( ngl_array_, ngl_array_name ) 
#define ngl_array_iter_struct join_token ( join_token ( ngl_array_, ngl_array_name ), _iter )

#define pointer 0xefaced
#if ngl_array_elem_kind == pointer
  #define ngl_array_cast_elem( elem ) ((ngl_val) { .ngl_array_elem_kind = ( ngl_obj * ) elem })
#else
  #define ngl_array_cast_elem( elem ) ((ngl_val) { .ngl_array_elem_kind = elem })
  #endif
#undef pointer

#define ngl_array_back_cast( val ) ( ( ngl_array_elem_t ) ( val ).ngl_array_elem_kind )

def_struct ( ngl_array_struct ) {
  ngl_array array;
  };

def_struct ( ngl_array_iter_struct ) {
  ngl_array_iter iter;
  };
static ngl_error * method( init ) ( ngl_array_struct * self ) {
  return ngl_array_init ( &self->array, ngl_array_elem_ngl_type );
  }

static ngl_error * method( init_to_size ) ( ngl_array_struct * self, long num_elems ) {
  return ngl_array_init_to_size ( &self->array, ngl_array_elem_ngl_type, num_elems );
  }


static ngl_array_struct * method ( new ) ( void ) {
  return ( ngl_array_struct * ) ngl_array_new ( ngl_array_elem_ngl_type );
  }

static ngl_array_struct * method ( new_to_size ) ( long num_elems ) {
  return ( ngl_array_struct * ) ngl_array_new_to_size ( ngl_array_elem_ngl_type, num_elems );
  }


static void method ( deinit ) ( ngl_array_struct * self ) {
  ngl_array_deinit ( &self->array );
  }

static void method ( delete ) ( ngl_array_struct * self ) {
  ngl_array_delete ( &self->array );
  }


static bool method ( empty ) ( ngl_array_struct * self ) {
  return ngl_array_empty ( &self->array );
  }

static bool method ( full ) ( ngl_array_struct * self ) {
  return ngl_array_full ( &self->array );
  }


static ngl_error * method ( push_back ) ( ngl_array_struct * self, ngl_array_elem_t elem ) {
  return ngl_array_push_back ( &self->array, ngl_array_cast_elem ( elem ) );
  }

static ngl_error * method ( push_front ) ( ngl_array_struct * self, ngl_array_elem_t elem ) {
  return ngl_array_push_front ( &self->array, ngl_array_cast_elem ( elem ) );
  }


static ngl_error * method ( pop_back ) ( ngl_array_struct * self, ngl_array_elem_t * elem ) {
  ngl_val elem_val;
  ngl_error * error = ngl_array_pop_back ( &self->array, &elem_val );
  if ( error == ngl_ok ) {
    *elem = ngl_array_back_cast ( elem_val );
    }
  return error;
  }

static ngl_error * method ( pop_front ) ( ngl_array_struct * self, ngl_array_elem_t * elem ) {
  ngl_val elem_val;
  ngl_error * error = ngl_array_pop_front ( &self->array, &elem_val );
  if ( error == ngl_ok ) {
    *elem = ngl_array_back_cast ( elem_val );
    }
  return error;
  }


static long method ( length ) ( ngl_array_struct * self ) {
  return ngl_array_length ( &self->array );
  }

static bool method ( good_index ) ( ngl_array_struct * self, long index ) {
  return ngl_array_good_index ( &self->array, index );
  }

static ngl_error * method ( index ) (
    ngl_array_struct * self,
    long index,
    ngl_array_elem_t ** elem
    ) {
  ngl_val * elem_val_ptr;
  ngl_error * error = ngl_array_index ( &self->array, index, &elem_val_ptr );
  if ( error == ngl_ok ) {
    *elem = ( ngl_array_elem_t * ) elem_val_ptr;
    }
  return error;
  }

static ngl_error * method ( remove ) ( ngl_array_struct * self, long index ) {
  return ngl_array_remove ( &self->array, index );
  }


static ngl_error * method ( get ) ( ngl_array_struct * self, long index, ngl_array_elem_t * elem ) {
  ngl_val elem_val;
  ngl_error * error = ngl_array_get ( &self->array, index, &elem_val );
  if ( error == ngl_ok ) {
    *elem = ngl_array_back_cast ( elem_val );
    }
  return error;
  }

static ngl_error * method ( set ) ( ngl_array_struct * self, long index, ngl_array_elem_t val ) {
  return ngl_array_set ( &self->array, index, ngl_array_cast_elem ( val ) );
  }


static ngl_array_iter_struct method ( start ) ( ngl_array_struct * parent ) {
  return (ngl_array_iter_struct) { ngl_array_start ( &parent->array ) };
  }

static ngl_array_iter_struct method ( end ) ( ngl_array_struct * parent ) {
  return (ngl_array_iter_struct) { ngl_array_end ( &parent->array ) };
  }


static void method ( prev ) ( ngl_array_iter_struct * self ) {
  ngl_array_prev ( &self->iter );
  }

static void method ( next ) ( ngl_array_iter_struct * self ) {
  ngl_array_next ( &self->iter );
  }


static bool method ( good ) ( ngl_array_iter_struct * self ) {
  return ngl_array_good ( &self->iter );
  }

static ngl_array_elem_t method ( deref ) ( ngl_array_iter_struct * self ) {
  return ngl_array_back_cast ( ngl_array_deref ( &self->iter ) );
  }

static ngl_array_elem_t * method ( at ) ( ngl_array_iter_struct * self, long rel_idx ) {
  return ( ngl_array_elem_t * )  ngl_array_at ( &self->iter, rel_idx );
  }

static bool method ( good_at ) ( ngl_array_iter_struct * self, long rel_idx ) {
  return ngl_array_good_at ( &self->iter, rel_idx );
  }

static ngl_error * method ( remove_at ) ( ngl_array_iter_struct * self, long rel_idx ) {
  return ngl_array_remove_at ( &self->iter, rel_idx );
  }




static long method ( index_of ) ( ngl_array_struct * self, ngl_array_elem_t to_find ) {
  return ngl_array_index_of ( &self->array, ngl_array_cast_elem ( to_find ) );
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
