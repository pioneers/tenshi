#define OP_NAME j_2
#define OP_NUM_ARGS 2

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  uint16_t u_offset = (arg_0.uinteger << 8) | arg_1.uinteger;
  pc += *(int16_t *) & u_offset;
  op_bunch = 0;
}
#endif

#include <ngl_opcode_end.c>
