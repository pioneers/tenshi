#include <assert.h>
#include <stdio.h>
#include <ngl_error.h>
#include <ngl_builtins.h>

#define ngl_array_elem_t ngl_int
#define ngl_array_elem_kind integer
#include <ngl_array.h>

int main ( ) {
  ngl_builtins_init ( );
  ngl_array_ngl_int * a = ngl_array_ngl_int_new ( );
  printf ( "a = %p\n", ( void * ) a );
  fflush ( stdout );
  printf ( "a->array.data = %p\n", ( void * ) a->array.data );
  return 0;
  }
