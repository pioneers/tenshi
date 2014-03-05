#ifndef NGL_BUFFER_H_H_
#define NGL_BUFFER_H_H_

#include <ngl_obj.h>
#include <ngl_val.h>
#include <stdint.h>

def_struct(ngl_buffer, {
  ngl_obj header;
  ngl_uint size;
  uint8_t first_byte;
})

#define NGL_BUFFER_DATA(buffer) ((void *) &(buffer)->first_byte)

#endif /* end of include guard: NGL_BUFFER_H_H_ */ /* NOLINT(*) */
