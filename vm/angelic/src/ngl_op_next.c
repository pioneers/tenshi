#define OP_NAME next

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
  /*
   * This opcode is effectively virtual, since we need to load the next bunch
   * from the tail of each opcode (located in ngl_opcode_end.c).
   */
#endif

#include <ngl_opcode_end.c>
