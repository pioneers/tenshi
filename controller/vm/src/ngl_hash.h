#ifndef NGL_HASH_H_
#define NGL_HASH_H_
#include <stdint.h>
#include <ngl_macros.h>
#include <ngl_type_h.h>
#include <ngl_obj.h>
#include <ngl_val.h>

#include <stdlib.h>             /* size_t */

typedef uint64_t hash_t;

def_struct(ngl_siphash_key, {
  uint64_t k[2];
})

hash_t siphash_24(const ngl_siphash_key key, const uint8_t * msg,
                  const size_t len);

typedef ngl_uint ngl_hash;
typedef ngl_hash(ngl_hash_func_t) (ngl_val);
typedef ngl_int(ngl_hash_compare_func_t) (ngl_val, ngl_val);

extern ngl_siphash_key ngl_hash_key;

ngl_hash ngl_hash_ngl_val(ngl_val val);
ngl_int ngl_compare_ngl_val(ngl_val a, ngl_val b);

#endif /* NGL_HASH_H_ */ /* NOLINT(*) */
