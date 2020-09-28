#pragma once

#include "entity.h"

class GameEntity : public Entity
{
public:
	GameEntity() : Entity() {
	}

	virtual void update(float timedelta) {};
	virtual void draw() {};
};