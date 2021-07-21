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

	FOWBuilding()
	{
	}

	FOWBuilding(int x, int y, int size)
	{
		type = FOW_BUILDING;
		position.x = x;
		position.y = y;
		this->size = size;

		skeleton = new spine::Skeleton(SpineManager::skeletonData["caterpillar"]);
		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();

		animationState = new spine::AnimationState(SpineManager::stateData["caterpillar"]);
		animationState->addAnimation(0, "idle_two", true, 0);
	}

	void draw()
	{
		glPushMatrix();
			glTranslatef(position.x-0.5, -position.y+0.5, 0.01f);
			SpineManager::drawSkeleton(skeleton);
		glPopMatrix();
	}

	int size;
};

class FOWTownHall: public FOWBuilding
{
public:

	FOWTownHall()
	{
	}

	FOWTownHall(int x, int y, int size)
	{
		type = FOW_TOWNHALL;

		skeleton = new spine::Skeleton(SpineManager::skeletonData["buildings"]);
		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();

		animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
		//animationState->addAnimation(0, "idle_two", true, 0);

		position.x = x;
		position.y = y;
		this->size = size;
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

	FOWGoldMine(int x, int z, int size)
	{
		type = FOW_GOLDMINE;
		//spine_data.load_spine_data("buildings");
		position.x = x;
		position.z = z;
		this->size = size;
	}
};