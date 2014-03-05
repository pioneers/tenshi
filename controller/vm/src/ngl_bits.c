#include <ngl_bits.h>

const void * ngl_end_args = ( void * ) ~ ( intptr_t )0;

/* Unaligned pointer value at end of memory space ( probably in kernel space ).
 * Used to indicate the last argument in a variable argument list of pointers
 * where NULL would indicate allocation failure.
 */

ngl_int ngl_round_up_power_2 ( ngl_int to_round ) {
  ngl_int next = 0;
  while ( ( next = ( to_round & ( to_round - 1 ) ) ) ) {
    to_round = next;
    }
  return to_round * 2;
  }

bool ngl_is_power_2 ( ngl_int input ) {
  return ! ( input & ( input - 1 ) );
  }

ngl_uint ngl_mask_of_pow2 ( ngl_uint val ) {
  ngl_uint result = 1;
  while ( val > 2 ) {
    result = result << 1 | result;
    val = val / 2;
    }
  return result;
  }
