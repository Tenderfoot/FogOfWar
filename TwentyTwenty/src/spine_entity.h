#pragma once

#include <string>
#include "game_entity.h"
#include "spine_manager.h"

class SpineEntity : public GameEntity
{
public:

	spine::Skeleton *skeleton;
	spine::AnimationState *animationState;

	SpineEntity() : GameEntity()
	{
		skeleton = nullptr;
		animationState = nullptr;
	}

	SpineEntity(std::string skin_name) : GameEntity()
	{
		skeleton = new spine::Skeleton(SpineManager::skeletonData);

		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();
		skeleton->setSkin("witch");

		animationState = new spine::AnimationState(SpineManager::stateData);
		animationState->addAnimation(0, "walk_two", true, 0);
	}

	void update(float timedelta) {
		animationState->update(timedelta);
		animationState->apply(*skeleton);
	};

	void draw() {
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		SpineManager::drawSkeleton(skeleton);
		glDisable(GL_BLEND);
	};
};