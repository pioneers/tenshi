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

#define OP_NAME j_3
#define OP_NUM_ARGS 3

#include <ngl_opcode_begin.c>

#ifdef OPCODE_BODY
{
  int32_t offset = arg_0.integer * (1 << 16);
  offset += arg_1.uinteger << 8;
  offset += arg_2.uinteger;
  pc += offset;
  op_bunch = 0;
}
#endif

#include <ngl_opcode_end.c>
