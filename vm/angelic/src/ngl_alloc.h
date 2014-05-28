#ifndef NGL_ALLOC_H_
#define NGL_ALLOC_H_
#include <stdlib.h>
#include <ngl_error.h>

/* TODO(kzentner): Make this conditionally use pvPortMalloc */
#define ngl_alloc_simple(type, count) ((type *) malloc(sizeof(type) * count))
#define ngl_free(ptr) (free(ptr))

extern ngl_error ngl_out_of_memory;

#endif  // NGL_ALLOC_H_
