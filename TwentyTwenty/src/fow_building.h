#pragma once

#include "fow_selectable.h"

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
		current_skin = new spine::Skin(*SpineManager::skeletonData["buildings"]->findSkin("GoldMine"));
		skeleton->setSkin(current_skin);
		position.x = x;
		position.y = y;
	}
};