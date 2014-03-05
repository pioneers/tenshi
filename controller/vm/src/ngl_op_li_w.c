#define OP_NAME li_w

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  ngl_stack_push ( &stack, * ( ngl_val * ) pc );
  pc += sizeof ( ngl_val ) / sizeof ( ngl_opbunch );
  #endif

#include <ngl_opcode_end.c>
