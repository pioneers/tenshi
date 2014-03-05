#ifndef NGL_ALLOC_H
#define NGL_ALLOC_H
#include <stdlib.h>
#include <ngl_error.h>

#define ngl_alloc_simple( type, count ) ( ( type * ) malloc ( sizeof ( type ) * count ) )
#define ngl_free( ptr ) ( free ( ptr ) )

extern ngl_error ngl_out_of_memory;

#endif /* end of include guard: NGL_ALLOC_H */
