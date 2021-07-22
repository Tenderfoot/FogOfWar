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
		color = t_vertex(1,1,1);

		skeleton = new spine::Skeleton(SpineManager::skeletonData["buildings"]);
		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();

		animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
	}

	void draw()
	{
		glPushMatrix();
			glTranslatef(position.x-0.5, -position.y+0.5, 0.01f);
			glColor3f(color.x, color.y, color.z);
			SpineManager::drawSkeleton(skeleton);
			glColor3f(1, 1, 1);
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

		position.x = x;
		position.y = y;
		this->size = size;
		color = t_vertex(1, 1, 1);
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

	FOWGoldMine(int x, int y, int size)
	{
		type = FOW_GOLDMINE;
		
		skeleton = new spine::Skeleton(SpineManager::skeletonData["buildings"]);
		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();

		animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
		
		position.x = x;
		position.y = y;
		this->size = size;
		color = t_vertex(1, 1, 1);
	}
};