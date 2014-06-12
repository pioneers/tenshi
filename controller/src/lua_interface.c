
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>
#include "ngl_types.h"   // NOLINT(build/include)

#include "inc/lua_interface.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/led_driver.h"



int lua_set_led(lua_State *L) {
  // ngl_error *ret = ngl_set_motor(lua_tonumber(L, 1), lua_tonumber(L, 2));
  // TODO(rqou): Um, I don't think this is how it's supposed to work?

  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(lua_tonumber(L, 1), 0b111);

  int ret = 1;  // 1 means ok.
  lua_pushnumber(L, ret);
  return 1;
}
