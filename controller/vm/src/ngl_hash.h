#ifndef NGL_HASH_H_
#define NGL_HASH_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>
#include <stdlib.h>             /* size_t */

#include <ngl_macros.h>
#include <ngl_obj.h>
#include <ngl_val.h>

hash_t siphash_24(const ngl_siphash_key key, const uint8_t * msg,
                  const size_t len);

extern ngl_siphash_key ngl_hash_key;

ngl_hash ngl_hash_ngl_val(ngl_val val);
ngl_int ngl_compare_ngl_val(ngl_val a, ngl_val b);

#endif  // NGL_HASH_H_
