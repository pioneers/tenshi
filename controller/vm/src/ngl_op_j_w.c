#define OP_NAME j4

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    pc += 1;
    pc += * (int32_t *) pc;
    op_bunch = 0;
  }
  #endif

#include <ngl_opcode_end.c>
