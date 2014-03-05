#ifndef NGL_UTF8_H

#define NGL_UTF8_H
#include <ngl_error.h>
#include <stdint.h>
#include <ngl_str.h>
#include <stdbool.h>

extern ngl_error ngl_invalid_utf8;

ngl_error * ngl_utf8_get ( uint32_t * dst, const char ** c, const char * past_end );

bool ngl_utf8_valid ( ngl_str * str );

#endif /* end of include guard: NGL_UTF8_H */
