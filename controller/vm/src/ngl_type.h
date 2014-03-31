#ifndef NGL_TYPE_H_
#define NGL_TYPE_H_

#include <ngl_types_wrapper.h>

#include <ngl_macros.h>
#include <stdlib.h>
#include <ngl_array_ngl_val.h>
#include <ngl_table_ngl_val.h>
#include <ngl_str.h>
#include <ngl_builtins.h>
#include <ngl_hash.h>

#define ngl_type_upcast(ptr_to_subtype) ((ngl_type *)(ptr_to_subtype))

ngl_declare_composite(ngl_type);

void ngl_type_init_header(ngl_type * t);

void ngl_type_init_base(ngl_type_base * t, ngl_str name, ngl_uint size);

extern ngl_type *ngl_type_ngl_type_alien;
extern ngl_type *ngl_type_ngl_type_base;
extern ngl_type *ngl_type_ngl_type_composite;
extern ngl_type *ngl_type_ngl_type_pointer;

#endif  // NGL_TYPE_H_
