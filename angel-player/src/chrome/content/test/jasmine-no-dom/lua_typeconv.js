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

const lua_emcc = require('tenshi/vendor-js/lua');
const lua_langsupp = require('tenshi/lang-support/lua');

let luaL_newstate = lua_emcc.cwrap('luaL_newstate', 'number');
let lua_close = lua_emcc.cwrap('lua_close', null, ['number']);
let lua_pushboolean = lua_emcc.cwrap('lua_pushboolean', null,
  ['number', 'number']);
let lua_pushinteger = lua_emcc.cwrap('lua_pushinteger', null,
  ['number', 'number']);
let lua_pushnil = lua_emcc.cwrap('lua_pushnil', null, ['number']);
let lua_pushnumber = lua_emcc.cwrap('lua_pushnumber', null,
  ['number', 'number']);
let lua_pushstring = lua_emcc.cwrap('lua_pushstring', null,
  ['number', 'string']);
let lua_pushlstring = lua_emcc.cwrap('lua_pushlstring', null,
  ['number', 'string', 'number']);
let lua_pushcclosure = lua_emcc.cwrap('lua_pushcclosure', null,
  ['number', 'number', 'number']);
let lua_getglobal = lua_emcc.cwrap('lua_getglobal', null,
  ['number', 'string']);
let luaL_loadbufferx = lua_emcc.cwrap('luaL_loadbufferx', 'number',
    ['number', 'string', 'number', 'string', 'string']);
let lua_pushlightuserdata = lua_emcc.cwrap('lua_pushlightuserdata', null,
  ['number', 'number']);
let lua_newuserdata = lua_emcc.cwrap('lua_newuserdata', null,
  ['number', 'number']);
let lua_pushthread = lua_emcc.cwrap('lua_pushthread', null,
  ['number']);
let lua_createtable = lua_emcc.cwrap('lua_createtable', null,
  ['number', 'number', 'number']);
let lua_settable = lua_emcc.cwrap('lua_settable', null,
  ['number', 'number']);
let lua_callk = lua_emcc.cwrap('lua_callk', null,
  ['number', 'number', 'number', 'number', 'number']);

describe("Lua/Javascript type conversion", function() {
  it("should convert boolean", function() {
    let L = luaL_newstate();
    lua_pushboolean(L, true);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(true);
  });
  it("should convert integer", function() {
    let L = luaL_newstate();
    lua_pushinteger(L, 42);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(42);
  });
  it("should convert nil", function() {
    let L = luaL_newstate();
    lua_pushnil(L);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(null);
  });
  it("should convert none (invalid)", function() {
    let L = luaL_newstate();
    let val = lua_langsupp.lua_to_js(L, 1000000);
    lua_close(L);

    expect(val).toEqual(undefined);
  });
  it("should convert number", function() {
    let L = luaL_newstate();
    // Be careful of what number you use here. If it can't be represented
    // exactly by a float the test might error (esp. since we're not using
    // precise 32-bit float everywhere and allowing some stuff to be converted
    // to double).
    lua_pushnumber(L, 1.5);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(1.5);
  });
  it("should convert string", function() {
    let L = luaL_newstate();
    lua_pushstring(L, "Kanade x Yuzuru");
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual("Kanade x Yuzuru");
  });
  it("should convert string with embedded null", function() {
    let L = luaL_newstate();
    lua_pushlstring(L, "Kanade \x00 Yuzuru", "Kanade \x00 Yuzuru".length);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual("Kanade \x00 Yuzuru");
  });
  it("should convert c function", function() {
    let L = luaL_newstate();
    lua_pushcclosure(L, 0x123, 0);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val.type).toEqual('cfunction');
    expect(val.func).toEqual(0x123);
  });
  it("should convert lua function", function() {
    let L = luaL_newstate();
    luaL_loadbufferx(L, "", 0, "", "t");
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val.type).toEqual('function');
    expect(val.func).not.toEqual(0);
  });
  it("should convert lightuserdata", function() {
    let L = luaL_newstate();
    lua_pushlightuserdata(L, 0x123);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val.type).toEqual('lightuserdata');
    expect(val.func).toEqual(0x123);
  });
  it("should convert full userdata", function() {
    let L = luaL_newstate();
    lua_newuserdata(L, 1);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val.type).toEqual('userdata');
    expect(val.func).not.toEqual(0);
  });
  it("should convert thread", function() {
    let L = luaL_newstate();
    lua_pushthread(L);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val.type).toEqual('thread');
    expect(val.func).toEqual(L);
  });
  it("should convert table with int key/value", function() {
    let L = luaL_newstate();
    lua_createtable(L, 0, 0);
    lua_pushinteger(L, 1);
    lua_pushinteger(L, 42);
    lua_settable(L, -3);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val[1]).toEqual(42);
  });
  it("should convert table with table value", function() {
    let L = luaL_newstate();
    lua_createtable(L, 0, 0);
    lua_pushinteger(L, 1);
    lua_createtable(L, 0, 0);
    lua_pushinteger(L, 1);
    lua_pushinteger(L, 42);
    lua_settable(L, -3);
    lua_settable(L, -3);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val[1][1]).toEqual(42);
  });

  it("should roundtrip convert string", function() {
    let L = luaL_newstate();
    lua_langsupp.js_to_lua(L, "Miku x Kaito");
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual("Miku x Kaito");
  });
  it("should roundtrip convert int", function() {
    let L = luaL_newstate();
    lua_langsupp.js_to_lua(L, 6969);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(6969);
  });
  it("should roundtrip convert float", function() {
    let L = luaL_newstate();
    lua_langsupp.js_to_lua(L, 1.125);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(1.125);
  });
  it("should roundtrip convert null", function() {
    let L = luaL_newstate();
    lua_langsupp.js_to_lua(L, null);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    expect(val).toEqual(null);
  });

  it("should be able to use luaify", function() {
    function testfunc(a, b, c) {
      return (a + b) + c;
    }
    let testfunc_lua = lua_langsupp.luaify(testfunc);
    let testfunc_ptr = lua_emcc.Runtime.addFunction(testfunc_lua);

    let L = luaL_newstate();
    lua_pushcclosure(L, testfunc_ptr);
    lua_langsupp.js_to_lua(L, 1);
    lua_langsupp.js_to_lua(L, 3);
    lua_langsupp.js_to_lua(L, "hi");
    lua_callk(L, 3, 1, 0, 0);
    let val = lua_langsupp.lua_to_js(L, -1);
    lua_close(L);

    lua_emcc.Runtime.removeFunction(testfunc_ptr);

    expect(val).toEqual("4hi");
  });
});
