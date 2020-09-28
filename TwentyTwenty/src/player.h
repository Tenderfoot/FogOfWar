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

	void init();

	void make_floor();

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