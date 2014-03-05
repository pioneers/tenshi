#ifndef NGL_VAL_H_

#define NGL_VAL_H_
#include <stdint.h>
#include <inttypes.h>

#include <ngl_type_h.h>
#include <ngl_macros.h>

#ifdef ANGELIC_MICRO
typedef int32_t ngl_int;
#define ngl_int_format "d"
typedef uint32_t ngl_uint;
#define ngl_uint_format "u"
typedef float ngl_float;
#define ngl_float_format "f"
#else
typedef int64_t ngl_int;
#define ngl_int_format PRId64
typedef uint64_t ngl_uint;
#define ngl_uint_format PRIu64
typedef double ngl_float;
#define ngl_float_format "d"
#endif

declare_struct(ngl_obj)

def_union(ngl_val, {
  ngl_int integer;
  ngl_uint uinteger;
  ngl_float floating;
  ngl_obj *pointer;
})

ngl_declare_base(ngl_val);

#define ngl_val_int(val) ((ngl_val) {.integer = (ngl_int)(val)})
#define ngl_val_uint(val) ((ngl_val) {.uinteger = (ngl_uint)(val)})
#define ngl_val_float(val) ((ngl_val) {.floating = (ngl_float)(val)})
#define ngl_val_pointer(val) ((ngl_val) {.pointer = (ngl_obj *)(val)})
#define ngl_val_addr(val_addr) ((ngl_val *) val_addr)

#endif /* end of include guard: NGL_VAL_H_ */ /* NOLINT(*) */
