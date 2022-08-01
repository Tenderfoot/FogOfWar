#pragma once

#include <string>
#include <lua/lua.hpp>

class ScriptManager
{
public:

	static void load_script(std::string script_name);

	// API for LUA
	static int howdy(lua_State* state);
	static int build_and_add_entity(lua_State* state);
	static int get_entities_of_type(lua_State* state);
	static int send_message(lua_State* state);
	static int get_buildings_for_team(lua_State* state);
	static int change_team(lua_State* state);
};