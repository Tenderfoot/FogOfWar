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
		animationState->addAnimation(0, "idle", true, 0);
	}

	virtual void update(float timedelta) {
		animationState->update(timedelta);
		animationState->apply(*skeleton);
	};

	virtual void draw() {
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glPushMatrix();
			glTranslatef(transform.x, transform.y, -50.0f);
			if (flip)
				glRotatef(180, 0.0f, 1.0f, 0.0f);
			SpineManager::drawSkeleton(skeleton);
		glPopMatrix();
		glDisable(GL_BLEND);
	};
};