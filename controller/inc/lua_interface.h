#ifndef INC_LUA_INTERFACE_H_
#define INC_LUA_INTERFACE_H_

#include <lua.h>
#include <lualib.h>



#define TEST_STATIC_LUA \
  "while get_digital(0)==0 do\n" /* Wait until sensors enumerate */\
  "end\n"\
  \
  "started = false\n"\
  "dead_zone = 20\n"\
  "range = 1024\n"\
  "midpoint = range/2\n"\
  "max_motor = 100\n"\
  \
  "while 1 do\n"                 /* Control loop */\
  "   sensor = get_analog(0)\n"\
  "   set_led(sensor)\n"\
  /*"   set_led((get_digital(0)%2)*2+(get_digital(1)%2)*4)\n"*/\
  /*"   set_digital(0,get_digital(1)*2)\n"*/\
  "   if not started and math.abs(sensor-midpoint)<=dead_zone then\n"\
  "      started = true\n"\
  "   end\n"\
  "   speed = 0\n"\
  \
  "   if started then\n"\
  "     speed = (sensor-midpoint)*max_motor/(range/2)\n"\
  "     \n"\
  "     \n"\
  "     \n"\
  "     \n"\
  "   end\n"\
  \
  "   set_motor(1,speed)\n"\
  "   set_motor(2,speed)\n"\
  "end\n"  /* NOLINT(*) */



void lua_register_all(lua_State *L);

int lua_set_led(lua_State *L);
int lua_get_button(lua_State *L);

int lua_get_digital(lua_State *L);
int lua_set_digital(lua_State *L);

int lua_get_analog(lua_State *L);
int lua_set_analog(lua_State *L);

int lua_set_motor(lua_State *L);



#endif  // INC_LUA_INTERFACE_H_

