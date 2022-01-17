#pragma once

#include "fow_selectable.h"

class FOWCharacter;

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
	
	void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
	void char_init();
	virtual void set_under_construction();
	virtual void take_damage(int amount);
	virtual void update(float time_delta);
	void construction_finished();
	void process_command(FOWCommand next_command);

	// this buildings skin
	std::string base_skin;

	// things for initial building construction
	float construction_start_time;
	float time_to_build;
	bool under_construction;
	FOWSelectable* builder;

	// for buildings that build units
	entity_types entity_to_build;
	FOWCharacter* last_built_unit;	// this is to give skeletons commands, probably a poor plan, should make something that returns
	bool can_build_units;
	int cost;
};

class FOWTownHall: public FOWBuilding
{
public:

	FOWTownHall(int x, int y) : FOWBuilding(x, y, 3)
	{
		type = FOW_TOWNHALL;
		base_skin = "TownHall";
		skin_name = "TownHall";
		time_to_build = 5000;
		can_build_units = true;
		entity_to_build = FOW_GATHERER;
	}
};

class FOWGoldMine : public FOWBuilding
{
public:

	FOWGoldMine(int x, int y) : FOWBuilding(x, y, 3)
	{
		type = FOW_GOLDMINE;
		base_skin = "GoldMine";
		skin_name = "GoldMine";
	}
};

class FOWFarm : public FOWBuilding
{
public:

	FOWFarm(int x, int y) : FOWBuilding(x, y, 2)
	{
		type = FOW_FARM;
		base_skin = "Farm";
		skin_name = "Farm";
		time_to_build = 5000;
	}
};

class FOWBarracks : public FOWBuilding
{
public:

	FOWBarracks(int x, int y) : FOWBuilding(x, y, 3)
	{
		type = FOW_BARRACKS;
		base_skin = "Barracks";
		skin_name = "Barracks";
		time_to_build = 5000;
		can_build_units = true;
		entity_to_build = FOW_KNIGHT;
	}
};

class FOWEnemySpawner : public FOWBuilding
{
public:

	FOWEnemySpawner(int x, int y) : FOWBuilding(x, y, 3)
	{
		type = FOW_ENEMYSPAWNER;
		base_skin = "Barracks";
		skin_name = "Barracks";
		time_to_build = 5000;
		last_spawn = SDL_GetTicks();
		entity_to_build = FOW_SKELETON;
	}

	// last time enemies were spawned;
	float last_spawn;

	// spawn enemies in update
	virtual void update(float time_delta);
};