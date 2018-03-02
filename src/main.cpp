#include <lua.hpp> 

namespace lml
{
	int parse(lua_State* L);
}

extern "C" __declspec(dllexport)
int luaopen_lml(lua_State* L)
{
	luaL_checkversion(L);
	lua_pushcfunction(L, lml::parse);
	return 1;
}
