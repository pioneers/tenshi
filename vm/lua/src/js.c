#ifdef EMSCRIPTEN
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <emscripten.h>
#include <assert.h>

/*
 * Call a javascript function from Lua, converting arguments in the process.
 * This is the fast version, which only converts numbers and (ascii) strings.  
 * Everything else is "converted" as a wrapper object.
 * Unicode strings are not supported by this function.
 *
 * This function conforms to the Lua C API, and expects at least one argument 
 * (the JavaScript function to call).
 */
int fastCallJSFunction(lua_State *L) {
  /* TODO(kzentner): Finish the non-fast version of this function. */
  int numArgs = lua_gettop(L);
  if (numArgs < 1 || !lua_islightuserdata(L, -numArgs)) {
    /*
     * First argument should be a JavaScript function, which we encode as a 
     * lightuserdata. If the first argument is some lightuserdata which is not 
     * a JavaScript function, we will probably crash at the end of this 
     * function.
     */
    return luaL_error(L, "fastCallJSFunction called incorrectly.");
  }
  
  /* TODO(kzentner): Figure out how to make this not a global. */
  EM_ASM(fastCallJSFunctionArgs = [];);
  for (int i = -numArgs + 1; i < 0; i++) {
    if (lua_isboolean(L, i)) {
      if (lua_toboolean(L, i)) {
        EM_ASM(fastCallJSFunctionArgs.push(true));
      } else {
        EM_ASM(fastCallJSFunctionArgs.push(false));
      }
    } else if (lua_iscfunction(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'cfunction';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_tocfunction(L, i));
    } else if (lua_isfunction(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'function';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_topointer(L, i));
    } else if (lua_isinteger(L, i)) {
      EM_ASM_INT({
        fastCallJSFunctionArgs.push($0);
      }, lua_tointeger(L, i));
    } else if (lua_islightuserdata(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'lightuserdata';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_topointer(L, i));
    } else if (lua_isnil(L, i)) {
      EM_ASM(fastCallJSFunctionArgs.push(null));
    } else if (lua_isnone(L, i)) {
      EM_ASM(fastCallJSFunctionArgs.push(undefined));
    } else if (lua_isnumber(L, i)) {
      EM_ASM_INT({
        fastCallJSFunctionArgs.push($0);
      }, lua_tonumber(L, i));
    } else if (lua_isstring(L, i)) {
      size_t cstr_len = 0;
      const char * cstr = lua_tolstring(L, i, &cstr_len);
      EM_ASM_INT({
        fastCallJSFunctionArgs.push(String.fromCharCode.apply(null, 
            Module.HEAP8.slice($0, $0 + $1)));
      }, cstr, cstr_len);
    } else if (lua_istable(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'table';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_topointer(L, i));
    } else if (lua_isthread(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'thread';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_tothread(L, i));
    } else if (lua_isuserdata(L, i)) {
      EM_ASM_INT({
        var a = {};
        a['type'] = 'userdata';
        a['func'] = $0;
        fastCallJSFunctionArgs.push(a);
      }, lua_topointer(L, i));
    } else {
      assert(0 && "Unknown type on lua stack!\n");
    }
  }
  int encountered_error = EM_ASM_INT({
    /*
     * Function "pointers" referring to external functions are indices in
     * an array, which are desnsely packed in functionPointers, starting at
     * index 0.  However, the function pointer values are 2, 4, 6, 8, etc.
     * This implies the following mapping.
     */
    var func_num = ($0 / 2) - 1;
    var the_function = Module.Runtime.functionPointers[func_num];

    try {
      fastCallJSFunctionResult = the_function.apply(null, fastCallJSFunctionArgs);
      return 0;
    } catch (_) {
      return 1;
    }
  }, lua_topointer(L, -numArgs));
  if (encountered_error) {
    /* There was an error! */
    /* TODO(kzentner): Copy error message from JavaScript back to Lua. */
    return luaL_error(L, "An error occured in JavaScript");
  } else {
    lua_pop(L, numArgs);
    lua_pushnumber(L, EM_ASM_DOUBLE({ return fastCallJSFunctionResult; }, 0));
    return 1;
  }
}

/*
 * Register all functions in this module as globals.
 */
void registerJS(lua_State *L) {
  lua_register(L, "fastCallJSFunction", fastCallJSFunction);
}
#endif
