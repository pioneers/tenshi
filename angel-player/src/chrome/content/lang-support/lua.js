/* jshint globalstrict: true */
"use strict";

let lua;

let luaL_newstate,
    luaL_loadbufferx,
    lua_tolstring,
    lua_tolstring_ptr,
    lua_dump,
    lua_close,
    lua_type,
    lua_toboolean,
    lua_isinteger,
    lua_tointegerx,
    lua_isnumber,
    lua_tonumberx,
    lua_isstring,
    lua_iscfunction,
    lua_tocfunction,
    lua_topointer,
    lua_isuserdata,
    lua_touserdata,
    lua_tothread;

const LUA_TNONE           = (-1);
const LUA_TNIL            = 0;
const LUA_TBOOLEAN        = 1;
const LUA_TLIGHTUSERDATA  = 2;
const LUA_TNUMBER         = 3;
const LUA_TSTRING         = 4;
const LUA_TTABLE          = 5;
const LUA_TFUNCTION       = 6;
const LUA_TUSERDATA       = 7;
const LUA_TTHREAD         = 8;

try {
  lua = require('tenshi/vendor-js/lua');

  luaL_newstate = lua.cwrap('luaL_newstate', 'number');
  luaL_loadbufferx = lua.cwrap('luaL_loadbufferx', 'number',
    ['number', 'string', 'number', 'string', 'string']);
  lua_tolstring = lua.cwrap('lua_tolstring', 'string',
    ['number', 'number', 'number']);
  lua_tolstring = lua.cwrap('lua_tolstring', 'number',
    ['number', 'number', 'number']);
  lua_dump = lua.cwrap('lua_dump', 'number',
    ['number', 'number', 'number']);
  lua_close = lua.cwrap('lua_close', null, ['number']);
  lua_type = lua.cwrap('lua_type', 'number', ['number', 'number']);
  lua_toboolean = lua.cwrap('lua_toboolean', 'number', ['number', 'number']);
  lua_isinteger = lua.cwrap('lua_isinteger', 'number', ['number', 'number']);
  lua_tointegerx = lua.cwrap('lua_tointegerx', 'number',
    ['number', 'number', 'number']);
  lua_isnumber = lua.cwrap('lua_isnumber', 'number', ['number', 'number']);
  lua_tonumberx = lua.cwrap('lua_tonumberx', 'number',
    ['number', 'number', 'number']);
  lua_isstring = lua.cwrap('lua_isstring', 'number', ['number', 'number']);
  lua_iscfunction = lua.cwrap('lua_iscfunction', 'number',
    ['number', 'number']);
  lua_tocfunction = lua.cwrap('lua_tocfunction', 'number',
    ['number', 'number']);
  lua_topointer = lua.cwrap('lua_topointer', 'number', ['number', 'number']);
  lua_isuserdata = lua.cwrap('lua_isuserdata', 'number', ['number', 'number']);
  lua_touserdata = lua.cwrap('lua_touserdata', 'number', ['number', 'number']);
  lua_tothread = lua.cwrap('lua_tothread', 'number', ['number', 'number']);
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

// Converts the item at index i on the Lua stack of L into a JavaScript object.
// Currently this only supports numbers and strings -- everything else is
// converted into a wrapper object.
exports.lua_to_js = function(L, i) {
  if (lua_type(L, i) == LUA_TBOOLEAN) {
    if (lua_toboolean(L, i)) {
      return true;
    } else {
      return false;
    }
  } else if (lua_isinteger(L, i)) {
    return lua_tointegerx(L, i, 0);
  } else if (lua_isnumber(L, i)) {
    return lua_tonumberx(L, i, 0);
  } else if (lua_type(L, i) == LUA_TNIL) {
    return null;
  } else if (lua_type(L, i) == LUA_TNONE) {
    return undefined;
  } else if (lua_isstring(L, i)) {
    // TODO(rqou): Do I have to hardcode 4?
    let strlen_addr = lua._malloc(4);
    let str_addr = lua_tolstring(L, i, strlen_addr);
    let strlen = lua.getValue(strlen_addr, 'i32');
    let str = String.fromCharCode.apply(null,
      lua.HEAPU8.subarray(str_addr, str_addr + strlen));
    lua._free(strlen_addr);
    return str;
  } else if (lua_iscfunction(L, i)) {
    return {
      'type': 'cfunction',
      'func': lua_tocfunction(L, i)
    };
  } else if (lua_type(L, i) == LUA_TFUNCTION) {
    // Note: must come after cfunction
    return {
      'type': 'function',
      'func': lua_topointer(L, i)
    };
  } else if (lua_type(L, i) == LUA_TLIGHTUSERDATA) {
    return {
      'type': 'lightuserdata',
      'func': lua_touserdata(L, i)
    };
  } else if (lua_isuserdata(L, i)) {
    // Note: must come after lightuserdata
    return {
      'type': 'userdata',
      'func': lua_touserdata(L, i)
    };
  } else if (lua_type(L, i) == LUA_TTHREAD) {
    return {
      'type': 'thread',
      'func': lua_tothread(L, i)
    };
  } else if (lua_type(L, i) == LUA_TTABLE) {
    return {
      'type': 'table',
      'func': lua_topointer(L, i)
    };
  } else {
    throw new Error("Unknown type on lua stack!");
  }
};
