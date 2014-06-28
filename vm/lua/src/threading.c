#include <threading.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/*
 * This module implements utilities used to implement non-cooperative threads / 
 * Actors. See the header for further documentation.
 */

/* Number of internal Lua operations per tick. */
const static int threading_ops_per_tick = 128;

static lua_Hook threading_old_hook = NULL;

void threading_yield_hook(lua_State *L,
                       lua_Debug *ar) {
  if (ar->event & LUA_HOOKCOUNT) {
    lua_yield(L, 0);
  } else if (threading_old_hook != NULL) {
    (*threading_old_hook)(L, ar);
  }
}

const char *threading_setup(lua_State *L) {
  threading_old_hook = lua_gethook(L);
  lua_sethook(L, &threading_yield_hook, lua_gethookmask(L) | LUA_MASKCOUNT, 
      threading_ops_per_tick);
  return NULL;
}

const char *threading_run_ticks(lua_State *L, int tick_count) {
  int status = 0;
  int ticks_so_far = 0;
  do {
    status = lua_resume(L, 0, 0);
    ticks_so_far += 1;
  } while (ticks_so_far < tick_count && status == LUA_YIELD);
  if (status == LUA_OK) {
    return "Done";
  } else if (status == LUA_YIELD) {
    /* TODO(kzentner): Consider returning "Yield" instead. */
    return NULL;
  } else {
    /* TODO(kzentner): Return a more specific error message. */
    return "Error";
  }
}
