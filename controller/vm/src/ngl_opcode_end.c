#ifdef OPCODE_LABEL
for (ngl_uint i = 0; i < ngl_stack_height(&stack); i++) {
  /* The following line is very useful for debugging. */
  /*printf("stack %03lu: 0x%016lx = %f\n",*/
         /*i,*/
         /*ngl_stack_get(&stack, i).uinteger,*/
         /*ngl_stack_get(&stack, i).floating);*/
}

op_bunch >>= 8 * OP_NUM_ARGS;
if (!op_bunch) {
  if (--ticks_remaining < 0) {
    if (ticks == -1) {
      ticks_remaining = NGL_INT_MAX;
    } else {
      goto exit;
    }
  }
  op_bunch = *pc;
  pc += 1;
}
#ifdef USE_COMPUTED_GOTOS
goto *jump_targets[op_bunch & 0xff];
#else
break;
#endif
#endif

#undef OP_NAME
#undef OP_NUM_ARGS
#undef OP_N
