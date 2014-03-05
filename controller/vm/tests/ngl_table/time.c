#include <assert.h>
#include <string.h>
#include <ngl_call.h>
#include <ngl_str.h>

#define ngl_table_key_t ngl_str_ptr
#define ngl_table_key_kind pointer
#define ngl_table_val_t ngl_str_ptr
#define ngl_table_val_kind pointer
#include <ngl_table.h>

int main ( ) {
  char * test_string = malloc ( sizeof( "test string" ) + 1 );
  assert ( test_string );
  strcpy ( test_string, "test string" );
  ngl_str * str = ngl_str_new ( test_string, NULL );
  ngl_table_ngl_str_ptr_to_ngl_str_ptr table;
  int iterations = 10 * 1000;
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init ( &table );
  for ( long long i = 0; i < iterations; i++ ) {
    ngl_table_ngl_str_ptr_to_ngl_str_ptr_set ( &table, str, str );
    }
  return 0;
  }
