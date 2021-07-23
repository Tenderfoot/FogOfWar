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

	int size;
};

class FOWTownHall: public FOWBuilding
{
public:

	FOWTownHall()
	{
	}

	FOWTownHall(int x, int y, int size) : FOWBuilding(x,y,size)
	{
		type = FOW_TOWNHALL;
	}

	void process_command(FOWCommand next_command)
	{
		
		if (next_command.type == BUILD_UNIT)
		{
			printf("Build Unit command recieved\n");
		}

		FOWSelectable::process_command(next_command);
	};

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
	}
};