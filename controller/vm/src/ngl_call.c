#include <ngl_call.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ngl_alloc.h>


/*
 * Should work like this:
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_argc 2
 * #include <ngl_call_declare.h>
 *
 * ... somewhere else ...
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_args NGL_ARG_INT ( 0, int ), NGL_ARG_PTR ( 1, char * )
 * #define ngl_call_argc 2
 * #include <ngl_call_define.c>
 *
 * ... somewhere else ...
 *
 * #define ngl_call_name to_wrap_func
 * #define ngl_call_ngl_types ngl_type_int, ngl_type_char_ptr
 * #include <ngl_call_init.c>
 *
 */


ngl_define_composite ( ngl_call );

ngl_error * ngl_call_push ( ngl_call * self, ngl_type * type, ngl_val val ) {
  if ( self->num_args > self->filled_args && type && type == self->arg_types[self->filled_args] ) {
    self->args[self->filled_args++] = val;
    return ngl_ok;
    }
  else {
    return &ngl_error_generic;
    }
  }

ngl_error * ngl_call_force_push ( ngl_call * self, ngl_val val ) {
  if ( self->num_args > self->filled_args ) {
    self->args[self->filled_args++] = val;
    return ngl_ok;
    }
  else {
    return &ngl_error_generic;
    }
  }

ngl_error * ngl_call_pop ( ngl_call * self, ngl_uint count ) {
  if ( self->filled_args >= count ) {
    self->filled_args -= count;
    return ngl_ok;
    }
  else {
    return &ngl_error_generic;
    }
  }

ngl_error * ngl_call_push_types ( ngl_call * self, ngl_uint count, ... ) {
  va_list types;
  va_start ( types, count );
  if ( !self || count > self->num_args ) {
    return &ngl_error_generic;
    }
  for ( ngl_uint i = 0; i < count; i++ ) {
    self->arg_types[i] = va_arg ( types, ngl_type * );
    }
  va_end ( types );
  return ngl_ok;
  }

ngl_error * ngl_invoke ( ngl_call * c ) {
  if ( c->func.header.type == ngl_type_ngl_ex_func ) {
    if ( c->func.ex_func.thunk == NULL ) {
      return ngl_ok;
      }
    if ( c->num_args != c->filled_args ) {
      return &ngl_error_generic;
      }
    else {
      return c->func.ex_func.thunk ( c->args );
      }
    }
  else {
    return &ngl_error_generic;
    }
  }

ngl_error * ngl_call_clone ( ngl_call * dest, ngl_call * src ) {
  ngl_val * new_args = ngl_alloc_simple ( ngl_val, src->num_args );
  if ( ! new_args ) {
    return &ngl_error_generic;
    }
  dest->func = src->func;
  dest->num_args = src->num_args;
  dest->filled_args = src->filled_args;
  dest->arg_types = src->arg_types;
  dest->args = new_args;
  memcpy ( new_args, src->args, src->filled_args * sizeof ( ngl_val ) );
  return ngl_ok;
  }

ngl_error * ngl_call_deinit ( ngl_call * c ) {
  if ( ! c ) {
    return &ngl_error_generic;
    }
  free ( c->args );
  *c = ngl_null_call;
  return ngl_ok;
  }

ngl_error * ngl_call_delete ( ngl_call * c ) {
  if ( ! c ) {
    return &ngl_error_generic;
    }
  free ( c->args );
  free ( c );
  return ngl_ok;
  }

void ngl_init_null_call ( ngl_call * call ) {
  ngl_obj_init ( &call->header, ngl_type_ngl_call );
  call->args = 0;
  ngl_ex_func_init ( &call->func, NULL );
  call->num_args = 0;
  call->filled_args = 0;
  call->arg_types = 0;
  }
