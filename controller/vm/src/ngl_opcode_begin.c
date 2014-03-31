#ifndef OP_NUM_ARGS
#define OP_NUM_ARGS 0
#endif

#ifdef OPCODE_ARGS
OP_NUM_ARGS
#endif

#ifdef OPCODE_TOKEN
  OP_NAME
#endif

#ifdef OPCODE_TARGET
  && join_token(target_, OP_NAME)
#endif

#ifdef OPCODE_NAME
  string_of_macro(OP_NAME)
#endif

#ifdef OPCODE_LIST
  ,
#endif

#ifdef OPCODE_LABEL
#ifdef USE_COMPUTED_GOTOS
join_token(target_, OP_NAME):
#else
  break;
case(OP_N):
  /* The following lines are very useful for debugging. */
#if 0
  printf("executing " string_of_macro(OP_NAME) "\n");
  printf("bunch 0x%016x\n", op_bunch);
  for (ngl_uint i = 0; i < ngl_stack_height(&stack); i++) {
    printf("stack 0x%016lx % 16.16g\n",
           0L + ngl_stack_get(&stack, i).uinteger,
           (double) ngl_stack_get(&stack, i).floating);
    }
#endif
#endif
#endif

#if OP_NUM_ARGS < 0 || OP_NUM_ARGS > OP_MAX_ARGS
#pragma message "Opcode " OP_NAME " has illegal number of arguments " \
                string_of_macro(OP_NUM_ARGS) "."
#pragma message "Should be 0 <= " string_of_macro(OP_NUM_ARGS) " <= " \
                string_of_macro(OP_MAX_ARGS) "."
#error Illegal number of arguments to opcode.
#endif

#ifdef OPCODE_BODY
op_bunch >>= 8;
#endif
