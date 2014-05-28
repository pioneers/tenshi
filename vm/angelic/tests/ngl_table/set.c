#include <string.h>
#include <ngl_call.h>
#include <ngl_str.h>

#define ngl_table_key_t ngl_str_ptr
#define ngl_table_key_kind pointer
#define ngl_table_val_t ngl_str_ptr
#define ngl_table_val_kind pointer
#include <ngl_table.h>


int main() {
  ngl_table_ngl_str_ptr_to_ngl_str_ptr table;
  const char * test_string = "this is the test string";
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init(&table);
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_set(
    &table,
    ngl_str_new(test_string, strchr(test_string, ' ')),
    ngl_str_new("this value", NULL));
  return 0;
  }
