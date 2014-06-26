
#include "inc/lua_interface.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/led_driver.h"
#include "inc/button_driver.h"

#define LUA_REGISTER(FUNC) lua_register(L, #FUNC, lua_ ## FUNC)



void lua_register_all(lua_State *L) {
  LUA_REGISTER(set_led);
  LUA_REGISTER(get_button);
  LUA_REGISTER(get_digital);
  LUA_REGISTER(set_digital);
}

int lua_set_led(lua_State *L) {
  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(lua_tonumber(L, 1), 0b111);

  int ret = 1;  // 1 means ok.
  lua_pushnumber(L, ret);
  return 1;
}

int lua_get_button(lua_State *L) {
  int button = button_driver_get_button_state(0);

  lua_pushnumber(L, button);
  return 1;
}

int lua_get_digital(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  lua_pushnumber(L, ss_get_digital_value(sensorIndex));
  return 1;
}
int lua_set_digital(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  uint8_t sensorVal = (uint8_t)lua_tonumber(L, 2);

  ss_set_digital_value(sensorIndex, sensorVal);

  lua_pushnumber(L, 1);  // 1 means ok.
  return 1;
}

