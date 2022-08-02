
#include "script_manager.h"
#include <lua/lua.hpp>
#include "entity.h"
#include "grid_manager.h"
#include "game.h"

extern lua_State* state;
static std::thread* script_thread{ nullptr };

extern bool is_unit(entity_types type);
extern bool is_building(entity_types type);

static void run_script_thread()
{
	lua_pcall(state, 0, LUA_MULTRET, 0);
}

void ScriptManager::load_script(std::string script_name)
{
	// register stuff to the API
	lua_register(state, "build_and_add_entity", build_and_add_entity);
	lua_register(state, "get_entities_of_type", get_entities_of_type);
	lua_register(state, "get_buildings_for_team", get_buildings_for_team);
	lua_register(state, "send_message", send_message);
	lua_register(state, "change_team", change_team);
	lua_register(state, "give_command", give_command);
	lua_register(state, "get_units_for_team", get_units_for_team);

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
	int type = lua_tointeger(state, 1);
	int x = lua_tointeger(state, 2);
	int y = lua_tointeger(state, 3);

	GridManager::build_and_add_entity((entity_types)type, t_vertex(x, y, 0.0f));

	return 0;
}

int ScriptManager::get_entities_of_type(lua_State* state)
{
	lua_pushinteger(state, 21);
	lua_pushinteger(state, 15);
	lua_pushinteger(state, 10);
	
	return 3;
}

int ScriptManager::give_command(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);
	int entity_id = lua_tointeger(state, 1);
	int command_type = lua_tointeger(state, 2);

	for (auto entity : Game::entities)
	{
		if (entity->id == entity_id)
		{
			if ((t_ability_enum)command_type == MOVE || (t_ability_enum)command_type == ATTACK_MOVE)
			{
				int pos_x = lua_tointeger(state, 3);
				int pos_y = lua_tointeger(state, 4);

				((FOWSelectable*)entity)->command_queue.push_back((FOWCommand((t_ability_enum)command_type, t_vertex(pos_x, pos_y, 0.0f))));
			}
			else
			{
				int command_target = lua_tointeger(state, 3);
				((FOWSelectable*)entity)->process_command(FOWCommand((t_ability_enum)command_type, (entity_types)command_target));
			}
		}
	}

	return 0;
}

int ScriptManager::change_team(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);
	int entity_id = lua_tointeger(state, 1);
	int new_team = lua_tointeger(state, 2);

	for (auto entity : Game::entities)
	{
		if (entity->id == entity_id)
		{
			((FOWSelectable*)entity)->team_id = new_team;
		}
	}

	return 0;
}

int ScriptManager::get_units_for_team(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);
	int team = lua_tointeger(state, 1);

	std::vector<GameEntity*> units;

	for (auto entity : Game::entities)
	{
		if (is_unit(entity->type))
		{
			if (((FOWCharacter*)entity)->team_id == team)
				units.push_back(entity);
		}
	}

	lua_pushinteger(state, units.size());

	for (auto unit : units)
	{
		lua_pushinteger(state, unit->id);
	}

	return units.size()+1;
}

int ScriptManager::get_buildings_for_team(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);	
	int team = lua_tointeger(state, 1);
	
	std::vector<GameEntity*> buildings;

	for (auto entity : Game::entities)
	{
		if (is_building(entity->type))
		{
			if (((FOWBuilding*)entity)->team_id == team)
				buildings.push_back(entity);
		}
	}

	for (auto building : buildings)
	{
		lua_pushinteger(state, building->id);
	}

	return buildings.size();
}

int ScriptManager::send_message(lua_State* state)
{
	int args = lua_gettop(state);
	std::string message = lua_tostring(state, 1);
	Game::send_error_message(message);
	return 0;
}