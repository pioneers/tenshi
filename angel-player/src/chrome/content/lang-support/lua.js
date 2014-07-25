/* jshint globalstrict: true */
"use strict";

let lua;

let luaL_newstate,
    luaL_loadbufferx,
    lua_tolstring,
    lua_dump,
    lua_close;

try {
  lua = require('tenshi/vendor-js/lua');

  luaL_newstate = lua.cwrap('luaL_newstate', 'number');
  luaL_loadbufferx = lua.cwrap('luaL_loadbufferx', 'number',
    ['number', 'string', 'number', 'string', 'string']);
  lua_tolstring = lua.cwrap('lua_tolstring', 'string',
    ['number', 'number', 'number']);
  lua_dump = lua.cwrap('lua_dump', 'number',
    ['number', 'number', 'number']);
  lua_close = lua.cwrap('lua_close', null, ['number']);
} catch(e) {
  // OK, running in dev without build.sh
}

// Compile text as a Lua module and return an object like the following:
// {
//  success: true/false,
//  bytecode: UInt8Array (only if success)
//  error: string (only if failure)
// }
exports.compile_lua = function(text, filename) {
  filename = filename || '<input>';

  if (!lua) {
    return {
      "success":  false,
      "error":
        "Emscripten Lua not available. Running in dev without build.sh?"
    };
  }

  let L = luaL_newstate();
  if (L === 0) {
    return {
      "success":  false,
      "error":
        "Could not allocate new Lua state!"
    };
  }
  let ret = luaL_loadbufferx(L, text, text.length, filename, 't');
  if (ret !== 0) {
    // An error occurred
    if (ret !== 3) {
      // Not a syntax error!

      lua_close(L);

      return {
        "success":  false,
        "error":
          "luaL_loadbufferx error: " + ret
      };
    } else {
      // A syntax error
      let error = lua_tolstring(L, -1, 0);
      lua_close(L);

      return {
        "success":  false,
        "error":    error
      };
    }
  } else {
    // No error, so we have to extract the data out using lua_dump
    let lua_bytecode = new Uint8Array(0);
    let lua_dump_callback = function(L, p, sz, ud) {
      let lua_bytecode_new = new Uint8Array(lua_bytecode.length + sz);
      lua_bytecode_new.set(lua_bytecode);
      lua_bytecode_new.set(
          lua.HEAPU8.subarray(p, p + sz), lua_bytecode.length);
      lua_bytecode = lua_bytecode_new;
    };
    let lua_dump_callback_ptr = lua.Runtime.addFunction(lua_dump_callback);
    ret = lua_dump(L, lua_dump_callback_ptr, 0);
    if (ret !== 0) {
      // Error
      lua_close(L);
      lua.Runtime.removeFunction(lua_dump_callback_ptr);

      return {
        "success":  false,
        "error":    "Error calling lua_dump: " + ret
      };
    }
    // No error, done
    lua_close(L);
    lua.Runtime.removeFunction(lua_dump_callback_ptr);

    return {
      "success":  true,
      "bytecode": lua_bytecode
    };
  }
};
