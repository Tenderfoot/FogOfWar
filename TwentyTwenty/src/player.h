#pragma once

#include "spine_entity.h"
#include <math.h>

#define MAX_FALL_SPEED -3
#define FALL_SPEED 1

class Player : public SpineEntity
{
public:
	Player() : SpineEntity("witch")
	{
	}

	void init()
	{
		t_transform aabb = SpineManager::getAABB(skeleton);

		float width = abs(aabb.x - aabb.w);
		float height = abs(aabb.y - aabb.h);

		printf("dimensions: %f, %f\n", width, height);

		velocity.x = 0;
		velocity.y = 0;
	}

	virtual void update(float timedelta) {

		if (velocity.y > MAX_FALL_SPEED)
		{
			velocity.y -= FALL_SPEED * timedelta;
		}

		transform.x += velocity.x;
		transform.y += velocity.y;

		SpineEntity::update(timedelta);
	};
};