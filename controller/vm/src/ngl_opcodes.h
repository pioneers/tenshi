#ifndef NGL_OPCODES_H
#define NGL_OPCODES_H
#include <stdint.h>
#include <math.h>
#include <ngl_macros.h>
#include <ngl_func.h>
#include <assert.h>

def_union ( ngl_asm_arg ) {
  int8_t integer;
  uint8_t uinteger;
  };

typedef uint8_t ngl_asm_op;

#define OP_MAX_ARGS 3

#endif /* end of include guard: NGL_OPCODES_H */
