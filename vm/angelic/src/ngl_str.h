#ifndef NGL_STR_H_
#define NGL_STR_H_

#include <ngl_types_wrapper.h>

#include <stdio.h>
#include <stdint.h>

#include <ngl_hash.h>
#include <ngl_macros.h>
#include <ngl_table_ngl_val.h>

enum ngl_str_alloc_type {
  ngl_str_dynamic = 1,
  ngl_str_static = 2,
  ngl_str_gc = 3
} END

ngl_declare_composite(ngl_str);

#define ngl_str_lit(literal) \
  ngl_str_from_static(literal)

ngl_str ngl_str_from_static(const char *literal);

ngl_str ngl_str_from_dynamic(const char *dynamic);

ngl_str ngl_str_from_filename(ngl_str filename);

ngl_str ngl_str_from_file(FILE * file);

ngl_int ngl_str_compare(ngl_str a, ngl_str b);

void ngl_str_init_dynamic(ngl_str * to_init);

void ngl_str_print(ngl_str str);

void ngl_str_println(ngl_str to_print);

size_t ngl_str_size(const ngl_str str);

ngl_hash ngl_str_hash(ngl_str str);

ngl_str *ngl_str_new(const char *start, const char *past_end);
ngl_str *ngl_str_new_alloc(const char *start, const char *past_end,
                           enum ngl_str_alloc_type alloc_type);
void ngl_str_init(ngl_str * self, const char *start, const char *past_end);
void ngl_str_init_alloc(ngl_str * self, const char *start,
                        const char *past_end,
                        enum ngl_str_alloc_type alloc_type);

ngl_str *ngl_str_from_int64(int64_t input);

ngl_int ngl_compare_ngl_str_ptr(ngl_val v_str_a, ngl_val v_str_b);
ngl_hash ngl_hash_ngl_str_ptr(ngl_val str);

extern ngl_table_elem_interface ngl_str_table_i;

ngl_str *ngl_str_copy(ngl_str * original);

ngl_str ngl_str_empty(void);

void ngl_str_free(ngl_str * str);

#endif  // NGL_STR_H_
