#include <lua.h>
/*
 * This module implements utilities used to implement non-cooperative threads / 
 * Actors. It relies on using lua_sethook(), so calls to lua_sethook() must be 
 * careful to stil call the lua_Hook registered by threading_setup(), or this 
 * module will stop working.
 */

/*
 * Must be called on the root lua_State before any threads are run.
 * Before this function is called, no other functions in this module will work 
 * as described. Always returns NULL. Interferes with any other lua_Hook from 
 * being called due to the count option (see lua_sethook() in the Lua manual 
 * for details).
 */
const char *threading_setup(lua_State *L);

/*
 * Runs a lua_State for some number of "ticks", an internally defined measure 
 * of processing. Returns one of three strings.
 * Returns "Done" if the thread executed.
 * Returns NULL if the thread yielded or was suspended.
 * Returns a string beginning with "Error" if the thread died due to an error.
 */
const char *threading_run_ticks(lua_State *L, int tick_count);
