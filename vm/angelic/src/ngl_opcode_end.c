// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

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
