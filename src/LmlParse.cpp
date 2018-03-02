#include "LmlParse.h" 

namespace lml
{
	int parse(lua_State* L)
	{
		lex l(luaL_checkstring(L, 1));
		const char* file = luaL_optstring(L, 2, "...");
		handler h(L);
		if (!l.parse(h)) {
			return luaL_error(L, "\n%s:%d: %s", file, (int)lua_tointeger(L, -2), lua_tostring(L, -1));
		}
		return 1;
	}
}
