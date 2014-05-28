#include <assert.h>
#include <stdio.h>
#include <ngl_error.h>
#include <ngl_builtins.h>

#define ngl_array_elem_t ngl_int
#define ngl_array_elem_kind integer
#include <ngl_array.h>

int main() {
  ngl_builtins_init();
  ngl_error * e = ngl_ok;
  ngl_array_ngl_int * a = ngl_array_ngl_int_new();
  ngl_array_ngl_int_iter i = ngl_array_ngl_int_start(a);
  ngl_err(e, ngl_array_ngl_int_push_front(a, 1));
  ngl_err(e, ngl_array_ngl_int_remove_at(&i, 0));
  assert(! ngl_array_ngl_int_good(&i));
  assert(ngl_array_ngl_int_length(a) == 0);
  return e != ngl_ok;
  }
