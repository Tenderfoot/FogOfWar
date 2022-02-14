#pragma once

#include "fow_selectable.h"

class FOWCharacter;
class UIProgressBar;

enum building_types
{
	BUILDING,
	TOWNHALL,
	GOLDMINE
};

class FOWBuilding : public FOWSelectable
{
public:

	FOWBuilding(int x, int y, int size);
	
	t_transform get_aabb();	// this is for selection
	void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
	void char_init();
	virtual void set_under_construction();
	virtual void take_damage(int amount);
	virtual void update(float time_delta);
	void construction_finished();
	void process_command(FOWCommand next_command);
	virtual void clear_selection();
	float get_depth();

	// this buildings skin
	std::string base_skin;

	// things for making units
	// - why is order important here??????? 
	bool can_build_units;
	bool currently_making_unit;
	float unit_start_time;
	float time_to_build_unit;
	static std::map<entity_types, int> unit_cost;

	// things for initial building construction
	float construction_start_time;
	float time_to_build;
	bool under_construction;
	FOWSelectable* builder;
	int gold_cost;
	int wood_cost;
	bool destroyed;

	// for buildings that build units
	static UIProgressBar* progress_bar;
	entity_types entity_to_build;
	FOWCharacter* last_built_unit;	// this is to give skeletons commands, probably a poor plan, should make something that returns
};

class FOWTownHall: public FOWBuilding
{
public:

	FOWTownHall(t_vertex position) : FOWBuilding(position.x, position.y, 3)
	{
		type = FOW_TOWNHALL;
		base_skin = "TownHall";
		skin_name = "TownHall";
		
		// base unit stats
		time_to_build = 55000;
		maximum_hp = 1200;
		current_hp = maximum_hp;
		gold_cost = 1200;
		wood_cost = 800;

		// unit build stats
		can_build_units = true;
		entity_to_build = FOW_GATHERER;
		time_to_build_unit = 12000;
	}
};

class FOWGoldMine : public FOWBuilding
{
public:

	FOWGoldMine(t_vertex position) : FOWBuilding(position.x, position.y, 3)
	{
		type = FOW_GOLDMINE;
		base_skin = "GoldMine";
		skin_name = "GoldMine";
	}
};

class FOWFarm : public FOWBuilding
{
public:

	FOWFarm(t_vertex position) : FOWBuilding(position.x, position.y, 2)
	{
		type = FOW_FARM;
		base_skin = "Farm";
		skin_name = "Farm";
		
		// base unit stats
		time_to_build = 25000;
		maximum_hp = 400;
		current_hp = maximum_hp;
		gold_cost = 500;
		wood_cost = 250;

		//can_build_units = true;
		//entity_to_build = FOW_ARCHER;
		//time_to_build_unit = 18000;
		//unit_cost = 500;
	}
};

class FOWBarracks : public FOWBuilding
{
public:

	FOWBarracks(t_vertex position) : FOWBuilding(position.x, position.y, 3)
	{
		type = FOW_BARRACKS;
		base_skin = "Barracks";
		skin_name = "Barracks";
		
		// base unit stats
		time_to_build = 50000;
		maximum_hp = 800;
		current_hp = maximum_hp;
		gold_cost = 700;
		wood_cost = 450;

		can_build_units = true;
		entity_to_build = FOW_KNIGHT;
		time_to_build_unit = 18000;
	}
};

class FOWEnemySpawner : public FOWBuilding
{
public:

	FOWEnemySpawner(t_vertex position) : FOWBuilding(position.x, position.y, 3)
	{
		type = FOW_ENEMYSPAWNER;
		base_skin = "Barracks";
		skin_name = "Barracks";
		time_to_build = 5000;
		last_spawn = SDL_GetTicks();
		entity_to_build = FOW_SKELETON;
		time_to_build_unit = 5000;
	}

	// last time enemies were spawned;
	float last_spawn;

	// spawn enemies in update
	virtual void update(float time_delta);
};