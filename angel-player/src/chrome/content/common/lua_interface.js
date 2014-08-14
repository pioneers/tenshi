var lua = require('lua');

function lua_add_function(lua_state, name, js_fn) {
  lua.ccall('lua_pushlightuserdata',
            'number', ['number', 'number'],
            [lua_state, lua.Runtime.addFunction(js_fn)]);
  lua.ccall('lua_setglobal',
            'number', ['number', 'string'],
            [lua_state, name]);
}



function make() {
  var lua_state = lua.ccall('luaL_newstate', 'number');
  // This adds needed functions from js.c into the global namespace,
  // including fastCallJSFunction
  lua.ccall('registerJS', 'number', ['number'], [lua_state]);

  var res = {
    lua_state : lua_state,
    add_fn : function(name, js_fn) {
      lua_add_function(res.lua_state, name, js_fn);
    },
    load_text : function(text) {
      var retcode = lua.ccall('luaL_loadstring', 'number', ['number', 'string'], [res.lua_state, text]);
      if (retcode !== 0) {
        throw "Invalid code!";
      }
    },
    start : function() {
      lua.ccall('lua_pcallk', 'number',
                ['number', 'number', 'number', 'number', 'number', 'number'],
                [res.lua_state, 0, 0, 0, 0, 0]);
    }
  };
  return res;
}

exports.make = make;
