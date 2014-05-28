#ifndef NGL_OPCODES_H_
#define NGL_OPCODES_H_
#include <stdint.h>
#include <math.h>
#include <ngl_macros.h>
#include <assert.h>
#include <limits.h>
#include <ngl_stack_ngl_val.h>
#ifdef NGL_ARM
#include <inc/stm32f4xx.h>
#endif
#ifdef NGL_EMCC
#include <emscripten.h>
#endif

def_union(ngl_asm_arg, {
  int8_t integer;
  uint8_t uinteger;
})

typedef uint8_t ngl_asm_op;

#define OP_MAX_ARGS 3

#endif  // NGL_OPCODES_H_
