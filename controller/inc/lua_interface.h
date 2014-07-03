#ifndef INC_LUA_INTERFACE_H_
#define INC_LUA_INTERFACE_H_

#include <lua.h>
#include <lualib.h>



// #define TEST_STATIC_LUA \
  "while 1 do\n"\
  /*"   set_led(get_digital(0)*2+get_digital(1)*4)\n"*/\
  "   set_digital(0,get_digital(1)*2)\n"\
  "   set_digital(1,get_digital(0)*2)\n"\
  "end"  /* NOLINT(*) */



void lua_register_all(lua_State *L);

int lua_set_led(lua_State *L);
int lua_get_button(lua_State *L);
int lua_get_digital(lua_State *L);
int lua_set_digital(lua_State *L);



#endif  // INC_LUA_INTERFACE_H_

