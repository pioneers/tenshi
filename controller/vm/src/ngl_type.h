#ifndef NGL_TYPE_H
#define NGL_TYPE_H
#include <ngl_macros.h>
#include <stdlib.h>
#include <ngl_array_ngl_val.h>
#include <ngl_table_ngl_val.h>
#include <ngl_str.h>
#include <ngl_type_h.h>
#include <ngl_builtins.h>
#include <ngl_hash.h>

#define ngl_type_upcast( ptr_to_subtype ) ( ( ngl_type * ) ( ptr_to_subtype ) )

ngl_declare_composite ( ngl_type );


struct ngl_type {
  ngl_obj header;
  ngl_type * ptr_of;
  ngl_str name;
  ngl_uint size;
  };

struct ngl_type_alien {
  ngl_obj header;
  ngl_type * ptr_of;
  ngl_str name;
  ngl_uint size;
  };

struct ngl_type_base {
  ngl_obj header;
  ngl_type * ptr_of;
  ngl_str name;
  ngl_uint size;
  };

struct ngl_type_composite {
  ngl_obj header;
  ngl_type * ptr_of;
  ngl_str name;
  ngl_uint size;
  ngl_table * names_to_types;
  ngl_table * names_to_offsets;
  };

struct ngl_type_pointer {
  ngl_obj header;
  ngl_type * ptr_of;
  ngl_str name;
  ngl_uint size;
  ngl_type * subtype;
  };


void ngl_type_init_header ( ngl_type * t );

void ngl_type_init_base ( ngl_type_base * t, ngl_str name, ngl_uint size );

extern ngl_type * ngl_type_ngl_type_alien;
extern ngl_type * ngl_type_ngl_type_base;
extern ngl_type * ngl_type_ngl_type_composite;
extern ngl_type * ngl_type_ngl_type_pointer;

#endif /* end of include guard: NGL_TYPE_H */
