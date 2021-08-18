#pragma once

#include "fow_selectable.h"
#
enum building_types
{
	BUILDING,
	TOWNHALL,
	GOLDMINE
};

class FOWBuilding : public FOWSelectable
{
public:

	FOWBuilding();
	FOWBuilding(int x, int y, int size);
	void draw();
	void make_vbo()
	{
		VBO = SpineManager::make_vbo(skeleton);
		animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
		animationState->addAnimation(0, "animation", true, 0);
	}

	virtual void set_under_construction()
	{
		under_construction = true;
		construction_start_time = SDL_GetTicks();
		skin_name = skin_name.append("_UC");
		reset_skin();
	}

	virtual void take_damage(int amount)
	{
		printf("in building take damage\n");
	}

	virtual void update(float time_delta)
	{
		if (under_construction)
			if (SDL_GetTicks() - construction_start_time > time_to_build)
				construction_finished();
	}

	void construction_finished();

	float construction_start_time;
	float time_to_build;
	bool under_construction;
	FOWSelectable* builder;
};

class FOWTownHall: public FOWBuilding
{
public:

	FOWTownHall();
	FOWTownHall(int x, int y, int size);

	void process_command(FOWCommand next_command);
	void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
};

class FOWGoldMine : public FOWBuilding
{
public:

	FOWGoldMine()
	{
	}

	FOWGoldMine(int x, int y, int size) : FOWBuilding(x, y, size)
	{
		type = FOW_GOLDMINE;
		load_spine_data("buildings", "GoldMine");
		make_vbo();
	}
};

class FOWFarm : public FOWBuilding
{
public:

	FOWFarm()
	{
	}

	FOWFarm(int x, int y, int size) : FOWBuilding(x, y, size)
	{
		type = FOW_FARM;
		load_spine_data("buildings", "Farm");
		make_vbo();
		time_to_build = 5000;
	}
};

class FOWBarracks : public FOWBuilding
{
public:

	FOWBarracks()
	{
	}

	FOWBarracks(int x, int y, int size) : FOWBuilding(x, y, size)
	{
		type = FOW_BARRACKS;
		load_spine_data("buildings", "Barracks");
		make_vbo();

		time_to_build = 5000;
	}

	void process_command(FOWCommand next_command);
	void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
};