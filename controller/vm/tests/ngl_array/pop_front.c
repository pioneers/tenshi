#include <assert.h>
#include <stdio.h>
#include <ngl_error.h>
#include <ngl_builtins.h>

#define ngl_array_elem_t ngl_int
#define ngl_array_elem_kind integer
#include <ngl_array.h>

int main ( ) {
  ngl_builtins_init ( );
  ngl_array_ngl_int a;
  ngl_error * e = ngl_ok;
  ngl_array_ngl_int_init ( &a );
  ngl_int i = 100;
  ngl_err ( e, ngl_array_ngl_int_push_front ( &a, i ) );
  ngl_int i2 = 0;
  ngl_err ( e, ngl_array_ngl_int_pop_front ( &a, &i2 ) );
  assert ( i2 == 100 );
  return e != ngl_ok;
  }
