#ifndef NGL_VAL_H_
#define NGL_VAL_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>
#include <inttypes.h>

#include <ngl_macros.h>

#ifdef ANGELIC_64
#define ngl_int_format PRId64
#define ngl_uint_format PRIu64
#else
#define ngl_int_format "d"
#define ngl_uint_format "u"
#endif

ngl_declare_base(ngl_val);

#define ngl_val_int(val) ((ngl_val) {.integer = (ngl_int)(val)})
#define ngl_val_uint(val) ((ngl_val) {.uinteger = (ngl_uint)(val)})
#define ngl_val_float(val) ((ngl_val) {.floating = (ngl_float)(val)})
#define ngl_val_pointer(val) ((ngl_val) {.pointer = (ngl_obj *)(val)})
#define ngl_val_addr(val_addr) ((ngl_val *) val_addr)

#endif  // NGL_VAL_H_
