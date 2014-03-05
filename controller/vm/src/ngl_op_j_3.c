#define OP_NAME j_3
#define OP_NUM_ARGS 3

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  {
    int32_t offset = arg_0.integer * ( 1 << 16 );
    offset += arg_1.uinteger << 8;
    offset += arg_2.uinteger;
    pc += offset;
    op_bunch = 0;
  }
  #endif

#include <ngl_opcode_end.c>
