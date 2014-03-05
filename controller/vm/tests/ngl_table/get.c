#include <string.h>
#include <ngl_call.h>
#include <ngl_str.h>

#define ngl_table_key_t ngl_str_ptr
#define ngl_table_key_kind pointer
#define ngl_table_val_t ngl_str_ptr
#define ngl_table_val_kind pointer
#include <ngl_table.h>


int main ( ) {
  ngl_table_ngl_str_ptr_to_ngl_str_ptr table;
  const char * test_string = "this is the test string";
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init ( &table );
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_set (
    &table,
    ngl_str_new ( test_string, strchr ( test_string, ' ' ) ),
    ngl_str_new ( "this value", NULL ) );

  ngl_str * value = NULL;
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_get ( &table, ngl_str_new( "this", NULL ), &value );
  puts ( "table[\"this\"] = " );
  ngl_str_println ( *value );
  if ( ! value ) {
    return 1;
    }
  if ( ngl_str_compare ( *value, ngl_str_lit ( "this value" ) ) != 0 ) {
    return 1;
    }
  return 0;
  }
