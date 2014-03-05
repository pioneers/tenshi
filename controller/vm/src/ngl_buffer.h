#ifndef NGL_BUFFER_H_
#define NGL_BUFFER_H_

#include <ngl_macros.h>
#include <ngl_obj.h>
#include <stdio.h>
#include <stdlib.h>

#include <ngl_str.h>

ngl_buffer *ngl_buffer_alloc(ngl_uint bytes);
ngl_buffer *ngl_buffer_from_file(FILE * file);
ngl_buffer *ngl_buffer_from_filename(ngl_str filename);

ngl_declare_composite(ngl_buffer);

#endif  // NGL_BUFFER_H_
