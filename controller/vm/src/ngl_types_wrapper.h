#ifndef NGL_TYPES_WRAPPER_H_
#define NGL_TYPES_WRAPPER_H_

#include <stdlib.h>
#include <stdint.h>

#if defined(NGL_64_BIT)
typedef int64_t ngl_int;
#define NGL_INT_MAX INT64_MAX
#define ngl_int_format PRId64
typedef uint64_t ngl_uint;
#define NGL_UINT_MAX UINT64_MAX
#define ngl_uint_format PRIu64
typedef double ngl_float;
#elif defined(NGL_32_BIT)
typedef int32_t ngl_int;
#define NGL_INT_MAX INT32_MAX
#define ngl_int_format PRId32
typedef uint32_t ngl_uint;
#define NGL_UINT_MAX UINT32_MAX
#define ngl_uint_format PRIu32
typedef float ngl_float;
#else
#error "Define NGL_32_BIT or NGL_64_BIT."
#endif

#include <ngl_types.h>

#endif  // NGL_TYPES_WRAPPER_H_
