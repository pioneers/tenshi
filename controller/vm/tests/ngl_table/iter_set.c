#include <string.h>
#include <ngl_call.h>
#include <ngl_str.h>

#define ngl_table_key_t ngl_str_ptr
#define ngl_table_key_kind pointer
#define ngl_table_val_t ngl_str_ptr
#define ngl_table_val_kind pointer
#include <ngl_table.h>

int main () {
  ngl_table_ngl_str_ptr_to_ngl_str_ptr table;
  int iterations = 100;
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init (&table);
  for (long long i = 0; i < iterations; i++) {
    ngl_str * str = ngl_str_from_llong(i);
    ngl_table_ngl_str_ptr_to_ngl_str_ptr_set (&table, str, str);
    }
  return 0;
  }
