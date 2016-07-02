// This file has been added to purposely expose the luaB_* functions so that
// it is possible to build a custom base module.

#ifndef LBASELIB_H_
#define LBASELIB_H_

LUAMOD_API int luaB_print (lua_State *L);
LUAMOD_API int luaB_tonumber (lua_State *L);
LUAMOD_API int luaB_error (lua_State *L);
LUAMOD_API int luaB_getmetatable (lua_State *L);
LUAMOD_API int luaB_setmetatable (lua_State *L);
LUAMOD_API int luaB_rawequal (lua_State *L);
LUAMOD_API int luaB_rawlen (lua_State *L);
LUAMOD_API int luaB_rawget (lua_State *L);
LUAMOD_API int luaB_rawset (lua_State *L);
LUAMOD_API int luaB_collectgarbage (lua_State *L);
LUAMOD_API int luaB_type (lua_State *L);
LUAMOD_API int luaB_next (lua_State *L);
LUAMOD_API int luaB_pairs (lua_State *L);
LUAMOD_API int luaB_ipairs (lua_State *L);
LUAMOD_API int luaB_loadfile (lua_State *L);
LUAMOD_API int luaB_load (lua_State *L);
LUAMOD_API int luaB_dofile (lua_State *L);
LUAMOD_API int luaB_assert (lua_State *L);
LUAMOD_API int luaB_select (lua_State *L);
LUAMOD_API int luaB_pcall (lua_State *L);
LUAMOD_API int luaB_xpcall (lua_State *L);
LUAMOD_API int luaB_tostring (lua_State *L);

#endif  // LBASELIB_H_
