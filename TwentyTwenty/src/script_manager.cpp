
#include "script_manager.h"
#include <lua/lua.hpp>
#include "entity.h"
#include "grid_manager.h"

extern lua_State* state;
static std::thread* script_thread{ nullptr };

static void run_script_thread()
{
	lua_pcall(state, 0, LUA_MULTRET, 0);
}

void ScriptManager::load_script(std::string script_name)
{
	// register stuff to the API
	lua_register(state, "build_and_add_entity", build_and_add_entity);

	// Load the program; this supports both source code and bytecode files.
	int result = luaL_loadfile(state, script_name.c_str());
	if (result != LUA_OK)
	{
		const char* message = lua_tostring(state, -1);
		printf(message);
		lua_pop(state, 1);
		return;
	}

	// execute the script
	script_thread = new std::thread(run_script_thread);
}

int ScriptManager::build_and_add_entity(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);

	printf("build_and_add_entity() was called with %d arguments:\n", args);

	for (int n = 1; n <= args; ++n) {
		printf("  argument %d: '%s'\n", n, lua_tostring(state, n));
	}
	int type = lua_tointeger(state, 1);
	int x = lua_tointeger(state, 2);
	int y = lua_tointeger(state, 3);

	GridManager::build_and_add_entity((entity_types)type, t_vertex(x, y, 0.0f));

	return 0;
}