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

const lua = require('tenshi/lang-support/lua');

describe("Lua smoke test", function() {
  it("should compile empty string", function() {
    let compile_result = lua.compile_lua("", "");
    expect(compile_result.success).toEqual(true);

    // Manually compiled using luac
    let reference_bytecode = [
      0x1b, 0x4c, 0x75, 0x61, 0x53, 0x00, 0x19, 0x93,
      0x0d, 0x0a, 0x1a, 0x0a, 0x04, 0x04, 0x04, 0x04,
      0x04, 0x78, 0x56, 0x00, 0x00, 0x00, 0x40, 0xb9,
      0x43, 0x01, 0x08, 0x3c, 0x69, 0x6e, 0x70, 0x75,
      0x74, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00,
      0x00, 0x26, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x05, 0x5f, 0x45, 0x4e, 0x56,
    ];

    expect(compile_result.bytecode.length).toEqual(reference_bytecode.length);
    for (let i = 0; i < compile_result.bytecode.length; i++) {
      expect(compile_result.bytecode[i]).toEqual(reference_bytecode[i]);
    }
  });
});
