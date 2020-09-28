#pragma once

#include "spine_entity.h"
#include <math.h>

#define MAX_FALL_SPEED -3
#define ACCELERATION_DTG 1

class Player : public SpineEntity
{
public:
	Player() : SpineEntity("witch")
	{
		falling = true;
	}

	void init();
	void make_floor();
	virtual void update(float timedelta);
	bool falling;
};