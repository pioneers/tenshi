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
  int iterations = 1000;
  ngl_table_ngl_str_ptr_to_ngl_str_ptr_init(&table);
  for(long long i = 0; i < iterations; i++) {
    ngl_str * str = ngl_str_from_int64(i);
    ngl_table_ngl_str_ptr_to_ngl_str_ptr_set(&table, str, str);
    }

  for(long long i = 0; i < iterations; i++) {
    ngl_str * str = ngl_str_from_int64(i);
    ngl_str * value = NULL;
    ngl_table_ngl_str_ptr_to_ngl_str_ptr_get(&table, str, &value);
    if (0 != ngl_str_compare(
        *str, 
        *value)) {
      printf("ngl_rong = ");
      ngl_str_println(*value);
      return 1;
      }
    }
  return 0;
  }
