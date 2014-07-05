// This file has been added to purposely expose the strlib functions so that
// it is possible to build a custom strlib module.

#ifndef LSTRLIB_H_
#define LSTRLIB_H_

LUAMOD_API int str_len (lua_State *L);
LUAMOD_API int str_sub (lua_State *L);
LUAMOD_API int str_reverse (lua_State *L);
LUAMOD_API int str_lower (lua_State *L);
LUAMOD_API int str_upper (lua_State *L);
LUAMOD_API int str_rep (lua_State *L);
LUAMOD_API int str_byte (lua_State *L);
LUAMOD_API int str_char (lua_State *L);
LUAMOD_API int str_dump (lua_State *L);
LUAMOD_API int str_find (lua_State *L);
LUAMOD_API int str_match (lua_State *L);
LUAMOD_API int gmatch (lua_State *L);
LUAMOD_API int str_gsub (lua_State *L);
LUAMOD_API int str_format (lua_State *L);
LUAMOD_API int dumpint_l (lua_State *L);
LUAMOD_API int undumpint_l (lua_State *L);
LUAMOD_API int dumpfloat_l (lua_State *L);
LUAMOD_API int undumpfloat_l (lua_State *L);

#endif  // LSTRLIB_H_
