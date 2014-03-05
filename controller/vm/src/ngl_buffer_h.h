#ifndef NGL_BUFFER_H_H_
#define NGL_BUFFER_H_H_

#include <ngl_obj.h>
#include <ngl_val.h>
#include <stdint.h>
/*
 * This file exists because there would otherwise be at least one unresolved
 * circular dependency (depending on how you count):
 *   - ngl_buffer -> ngl_str -> ngl_call -> ngl_func -> ngl_buffer
 *   - ngl_buffer -> ngl_str -> ngl_call -> (other) -> ngl_func -> ngl_buffer
 *   ... probably more.
 */


def_struct(ngl_buffer, {
  ngl_obj header;
  ngl_uint size;
  uint8_t first_byte;
})

#define NGL_BUFFER_DATA(buffer) ((void *) &(buffer)->first_byte)

#endif  // NGL_BUFFER_H_H_
