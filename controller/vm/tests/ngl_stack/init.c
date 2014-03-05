#include <assert.h>
#include <ngl_builtins.h>
#include <ngl_error.h>
#include <ngl_stack_ngl_val.h>

int main ( ) {
  ngl_builtins_init ( );
  ngl_stack s;
  return ngl_ok != ngl_stack_init ( &s );
  }
