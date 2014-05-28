#include <assert.h>
#include <ngl_error.h>
#include <ngl_builtins.h>

#define ngl_array_elem_t ngl_int
#define ngl_array_elem_kind integer
#include <ngl_array.h>

int main() {
  ngl_builtins_init();
  ngl_array_ngl_int a;
  return ngl_ok != ngl_array_ngl_int_init(&a);
  }
