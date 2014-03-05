#ifdef OPCODE_TARGET
  #define OPCODE_LIST
  #endif

#define OP_N 0x00
#include <ngl_op_next.c>

#define OP_N 0x01
#include <ngl_op_dup_1.c>

#define OP_N 0x02
#include <ngl_op_li_w.c>

#define OP_N 0x03
#include <ngl_op_set_1.c>

#define OP_N 0x04
#include <ngl_op_pop.c>

#define OP_N 0x05
#include <ngl_op_call_1.c>

#define OP_N 0x06
#include <ngl_op_ret.c>

#define OP_N 0x07
#include <ngl_op_eq.c>

#define OP_N 0x08
#include <ngl_op_j_1.c>

#define OP_N 0x09
#include <ngl_op_j_2.c>

#define OP_N 0x0a
#include <ngl_op_j_3.c>

#define OP_N 0x0b
#include <ngl_op_j_w.c>

#define OP_N 0x0c
#include <ngl_op_bz_1.c>

#define OP_N 0x0d
#include <ngl_op_bz_2.c>

#define OP_N 0x0e
#include <ngl_op_bz_3.c>

#define OP_N 0x0f
#include <ngl_op_bz_w.c>

#define OP_N 0x10
#include <ngl_op_not.c>

#define OP_N 0x11
#include <ngl_op_fadd.c>

#define OP_N 0x12
#include <ngl_op_fsub.c>

#define OP_N 0x13
#include <ngl_op_fmul.c>

#define OP_N 0x14
#include <ngl_op_fdiv.c>

#define OP_N 0x15
#include <ngl_op_fmod.c>

#ifdef USE_COMPUTED_GOTOS
  #error "Computed gotos are broken currently." \
  "This is because the array generated here must be dense."
  #endif

/* TODO(kzentner): Change these to iadd, etc. */
/*#define OP_N 0x16*/

/*#define OP_N 0x17*/

/*#define OP_N 0x18*/

/*#define OP_N 0x19*/

/*#define OP_N 0x1a*/


/* TODO(kzentner): Change these to uadd, etc. */
/*#define OP_N 0x1b*/

/*#define OP_N 0x1c*/

/*#define OP_N 0x1d*/

/*#define OP_N 0x1e*/

/*#define OP_N 0x1f*/


#define OP_N 0x20
#include <ngl_op_refi.c>

#define OP_N 0x21
#include <ngl_op_refd.c>

/*#define OP_N 0x22*/
/* TODO(kzentner): ngl_op_make_1 */

/*#define OP_N 0x23*/
/* TODO(kzentner): ngl_op_pushfree_1 */

/*#define OP_N 0x24*/
/* TODO(kzentner): ngl_op_popfree_1 */

/*#define OP_N 0x25*/
/* TODO(kzentner): clone */

/*#define OP_N 0x26*/
/* TODO(kzentner): safe */

#define OP_N 0x27
#include <ngl_op_read.c>

#define OP_N 0x28
#include <ngl_op_write.c>

#define OP_N 0x29
#include <ngl_op_stack_1.c>

#define OP_N 0x2a
#include <ngl_op_noop.c>

#define OP_N 0x2b
#include <ngl_op_end.c>

#define OP_N 0x2c
#include <ngl_op_debug.c>

/*#define OP_N 0x2d*/
/* TODO(kzentner): f2i */

/*#define OP_N 0x2e*/
/* TODO(kzentner): i2f */

/*#define OP_N 0x2f*/
/* TODO(kzentner): band */

/*#define OP_N 0x30*/
/* TODO(kzentner): bor */

/*#define OP_N 0x31*/
/* TODO(kzentner): bxor */

/*#define OP_N 0x32*/
/* TODO(kzentner): bnot */

/*#define OP_N 0x33*/
/* TODO(kzentner): bsl */

/*#define OP_N 0x34*/
/* TODO(kzentner): bsrl */

/*#define OP_N 0x35*/
/* TODO(kzentner): bsra */

/*#define OP_N 0x36*/
/* TODO(kzentner): catch_w */

/*#define OP_N 0x37*/
/* TODO(kzentner): throw */

/* Must be before last opcode. */
#undef OPCODE_LIST

#define OP_N 0x1e
#include <ngl_op_last.c>

#if defined ( OPCODE_LABEL ) && !defined ( USE_COMPUTED_GOTOS )
  default:
  assert ( 0 );
  #endif

#undef OPCODE_TARGET
#undef OPCODE_LABEL
#undef OPCODE_BODY
#undef OPCODE_NAME
#undef OPCODE_ARGS
