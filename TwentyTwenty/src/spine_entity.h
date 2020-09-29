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

	t_transform get_aabb()
	{
		t_transform aabb;
		float x1 = transform.x - (transform.w / 2);
		float y1 = transform.y;
		float x2 = transform.x + (transform.w / 2);
		float y2 = transform.y + transform.h;

		aabb.x = std::min(x1, x2);
		aabb.w = std::max(x1, x2);
		aabb.y = std::min(y1, y2);
		aabb.h = std::max(y1, y2);

		return aabb;
	}

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