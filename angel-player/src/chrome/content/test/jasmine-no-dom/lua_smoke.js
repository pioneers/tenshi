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
      0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00, 0x26, 0x00, 0x80, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x05, 0x5f, 0x45, 0x4e, 0x56,
    ];

    expect(compile_result.bytecode.length).toEqual(reference_bytecode.length);
    for (let i = 0; i < compile_result.bytecode.length; i++) {
      expect(compile_result.bytecode[i]).toEqual(reference_bytecode[i]);
    }
  });
});
