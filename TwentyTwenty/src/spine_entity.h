#pragma once

#include <string>
#include "game_entity.h"
#include "spine_manager.h"

class SpineEntity : public GameEntity
{
public:

	spine::Skeleton *skeleton;
	spine::AnimationState *animationState;
	bool flip;
	t_vertex color;

	SpineEntity() : GameEntity()
	{
		skeleton = nullptr;
		animationState = nullptr;
		color = t_vertex(1, 1, 1);
	}

	SpineEntity(std::string skin_name) : GameEntity()
	{
		skeleton = new spine::Skeleton(SpineManager::skeletonData["spine"]);

		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();
		skeleton->setSkin(spine::String(skin_name.c_str()));

		animationState = new spine::AnimationState(SpineManager::stateData["spine"]);
		animationState->addAnimation(0, "idle_two", true, 0);
	}

	virtual void update(float timedelta);
	t_transform get_aabb();
	virtual void draw();
};