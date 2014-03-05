#include <ngl_globals.h>
#include <ngl_macros.h>

ngl_error * ngl_globals_init ( ngl_globals * self ) {
  ngl_ret_on_err ( ngl_array_init (
      &self->type_id_to_type_ptr,
      ngl_type_ngl_type_ptr ) );
  ngl_ret_on_err ( ngl_table_init (
      &self->obj_table,
      ngl_type_ngl_type_ptr,
      ngl_type_ngl_obj_ptr,
      &ngl_table_ngl_val_interface ) );
  return ngl_ok;
  }

ngl_error * ngl_globals_get_obj_from_ids (
    ngl_globals * self,
    uint32_t type_id,
    uint32_t id,
    ngl_obj ** res
  ) {
  ngl_type * type;
  ngl_error * e = ngl_array_get (
      &self->type_id_to_type_ptr,
      type_id,
      ngl_val_addr ( &type ) );
  if ( e != ngl_ok ) {
    return e;
    }
  return ngl_globals_get_obj ( self, type, id, res );
  }

ngl_error * ngl_globals_get_obj (
    ngl_globals * self,
    ngl_type * type,
    uint32_t id,
    ngl_obj ** res
  ) {
  ngl_array * array;
  ngl_error * e =  ngl_table_get (
      &self->obj_table,
      ngl_val_pointer ( type ),
      ngl_val_addr ( &array ) );
  if ( e != ngl_ok ) {
    return e;
    }
  for ( ngl_int i = 0; i < ngl_array_length ( array ); i++ ) {
    void * p;
    ngl_array_get ( array, i, ngl_val_addr ( &p ) );
    }
  return ngl_array_get ( array, id, ngl_val_addr ( res ) );
  }

ngl_error * ngl_globals_set_obj_from_ids (
    ngl_globals * self,
    uint32_t type_id,
    uint32_t id,
    ngl_obj * to_store
  ) {
  ngl_type * type;
  ngl_error * e = ngl_array_get (
      &self->type_id_to_type_ptr,
      type_id,
      ngl_val_addr ( &type ) );
  if ( e != ngl_ok ) {
    return e;
    }
  return ngl_globals_set_obj ( self, type, id, to_store );
  }

ngl_error * ngl_globals_set_obj (
    ngl_globals * self,
    ngl_type * type,
    uint32_t id,
    ngl_obj * to_store
  ) {
  ngl_array * array;
  ngl_error * e =  ngl_table_get (
      &self->obj_table,
      ngl_val_pointer ( type ),
      ngl_val_addr ( &array ) );
  if ( e != ngl_ok ) {
    return e;
    }
  for ( ngl_uint i = ngl_array_length ( array ); i <= id; i++ ) {
    ngl_array_push_back ( array, ngl_val_pointer ( NULL ) );
    }
  return ngl_array_set ( array, id, ngl_val_pointer ( to_store ) );
  }
