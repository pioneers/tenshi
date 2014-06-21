#ifdef EMSCRIPTEN
#include <lua.h>

void registerJS(lua_State *L);

int fastCallJSFunction(lua_State *L);
#endif
