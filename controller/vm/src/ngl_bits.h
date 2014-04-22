#ifndef NGL_BITS_H_
#define NGL_BITS_H_

#include <stdint.h>
#include <stdbool.h>
#include <ngl_val.h>


extern const void *ngl_end_args;

ngl_int ngl_round_up_power_2(ngl_int to_round);

bool ngl_is_power_2(ngl_int input);

ngl_uint ngl_mask_of_pow2(ngl_uint val);

bool check_mask(ngl_uint mask);


#endif  // NGL_BITS_H_
